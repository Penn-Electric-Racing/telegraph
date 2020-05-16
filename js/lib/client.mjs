import { Signal } from './signal.mjs'
import { Node, Type, Value } from './nodes.mjs'
import { Adapter } from './adapter.mjs'
import { Collection } from './collection.mjs'
import { Connection, Params } from './connection.mjs'
import WebSocket from 'isomorphic-ws'
import { NamespaceQuery } from './query.mjs'
import { Component, Context, Namespace, Request } from './namespace.mjs'

function checkError(packet) {
  if (packet.error) throw new Error(packet.error);
  return packet;
}

// A client implements the namespace API
export class Client extends Namespace {
  constructor() {
    super();
    this._conn = null;

    this.contexts = new Collection();
    this.components = new Collection();
    this.mounts = new Collection();
  }

  query() { 
    var query = new NamespaceQuery(this);
    query.update(this);
    return query;
  }

  async connect(address) {
    if (this._conn) throw new Error('Already connected!');
    try {
      this._conn = new Connection(new WebSocket(address), true);
      this._conn.onClose.add(() => {
        this.contexts._clear();
        this.components._clear();
        this.mounts._clear();
        this._conn = null;
      });
      await this._conn.connect();
      await this._queryNS();
    } catch (e) {
      // if we couldn't connect, set connection to null and throw an error
      this._conn = null;
      throw e;
    }
  }

  async disconnect() {
    if (this._conn) {
      await this._conn.disconnect();
      this._conn = null;
    }
  }

  // will block until the client is disconnected
  wait() {
    return new Promise((res, rej) => {
      if (!this._conn.isOpen()) res();
      else this._conn.onClose.add(res);
    });
  }

  async _queryNS() {
    // send a queryNs
    var [nsRes, stream] = await this._conn.requestStream({queryNs: {}});
    stream.received.add((packet) => {
        // handle Context/Component/Mount added/removed
        if (packet.contextAdded) {
          let c = packet.contextAdded;
          this.contexts._add(new RemoteContext(this, c.uuid, c.name, c.type, Params.unpack(c.params)));

        } else if (packet.contextRemoved) {
          this.contexts._removeUUID(packet.contextRemoved);

        } else if (packet.componentAdded) {
          let t = packet.componentAdded;
          this.components._add(new RemoteComponent(this, t.uuid, t.name, t.type, Params.unpack(t.params)));
        } else if (packet.componentRemoved) {
          this.components._removeUUID(packet.componentRemoved);
        } else if (packet.mountAdded) {
          var m = packet.mountAdded;
          this.mounts._add({ uuid : m.src + '/' + m.tgt, 
                            src: this.contexts.get(m.src), 
                            tgt: this.contexts.get(m.tgt) });

        } else if (packet.mountRemoved) {
          var u = packet.mountRemoved.src + '/' + packet.mountRemoved.tgt;
          this.mounts._removeUUID(u);
        }
    });
    try {
      stream.start();

      checkError(nsRes);
      // populate the namespace based on the query
      if (!nsRes.ns) throw new Error("Malformed response!");
      for (let c of nsRes.ns.contexts) {
        this.contexts._add(new RemoteContext(this, c.uuid, c.name, c.type, Params.unpack(c.params)));
      }
      for (let t of nsRes.ns.components) {
        this.components._add(new RemoteComponent(this, t.uuid, t.name, t.type, Params.unpack(t.params)));
      }
      for (let m of nsRes.ns.mounts) {
        this.mounts._add({ uuid : m.src + '/' + m.tgt, 
            src: this.contexts.get(m.src), 
            tgt: this.contexts.get(m.tgt) });
      }

      // keep a reference to the stream so it stays
      // alive
      this._nsStream = stream;
      var onClosed = () => { this._nsStream = null; stream.remove(onClosed); }
      this._nsStream.closed.add(onClosed);
    } catch (e) {
      this._nsStream = null;
      stream.close();
      throw e;
    }
  }

  async createContext(name, type, params={}, srcs={}) {
    var convertedSrcs = []
    for (let [k, v] of Object.entries(srcs)) {
      if (v instanceof RemoteContext) {
        convertedSrcs.push({key: k, context: v.uuid});
      } else if (v instanceof Node) {
        convertedSrcs.push({key: k, root: v.pack()})
      } else {
        throw new Error("Invalid source!");
      }
    }
    var msg = {
      createContext : {
        name: name,
        type: type,
        params: Params.pack(params),
        sources: convertedSrcs
      }
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.payload == 'success' && res.success == false) return null;
    if (!res.contextCreated) throw new Error("unexpected response: " + JSON.stringify(res));
    return this.contexts.get(res.contextCreated);
  }

  async createComponent(name, type, params={}, srcs={}) {
    var convertedSrcs = []
    for (let [k, v] of Object.entries(srcs)) {
      if (v instanceof RemoteContext) {
        convertedSrcs.push({key: k, context: v.uuid});
      } else if (v instanceof Node) {
        convertedSrcs.push({key: k, root: v.pack()})
      } else {
        throw new Error("Invalid source!");
      }
    }
    var msg = {
      createComponent: {
        name: name,
        type: type,
        params: Params.pack(params),
        sources: convertedSrcs
      }
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.payload == 'success' && res.success == false) return null;
    if (!res.componentCreated) throw new Error("unexpected response: " + JSON.stringify(res));
    return this.components.get(res.componentCreated);
  }

  async destroyContext(uuid) {
    var msg = {
      destroyContext: uuid
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.success != true) 
      throw new Error("Failed to destroy context");
  }

  async destroyComponent(uuid) {
    var msg = {
      destroyComponent: uuid
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.success != true) 
      throw new Error("Failed to destroy component");
  }
}

class RemoteContext extends Context {
  constructor(ns, uuid, name, type, params) {
    super();
    this.ns = ns;
    this.uuid = uuid;
    this.name = name;
    this.type = type;
    this.params = params;
    this._adapters = new Map();
  }

  // will use locally-cached copy, re-fetch otherwise
  // TODO: make locally-chaced copy 
  // a WeakRef to not leak memory
  async fetch() {
    if (!this._cached_tree) {
      let conn = this.ns._conn;
      var msg = {
        fetchTree: this.uuid
      };
      this._cached_tree = new Promise((res,rej) => {
        conn.requestResponse(msg).then((response) => {
          checkError(response);
          if (response.payload == 'success' && 
              response.success == false) {
            this._cached_tree = null;
            res(null);
            return;
          }
          if (!response.fetchedTree) throw new Error("unexpected response: " + response);
          var tree = Node.unpack(response.fetchedTree);
          tree.setContext(this);
          res(tree);
        });
      });
      return await this._cached_tree;
    } else {
      return await this._cached_tree;
    }
  }

  async subscribe(variable, minInterval, maxInterval, timeout) {
    return await this.subscribePath(variable.path(), minInterval, maxInterval, timeout);
  }

  async subscribePath(path, minInterval, maxInterval, timeout) {
    var key = path.join('/');
    var adapter = this._adapters.get(key);
    if (!adapter) {
      console.log('creating new adapter!');
      var adapter_response = null;

      var adapter_stream = null;
      var adapter_type = null;

      adapter = new Adapter(
        // poll()
        () => {
          if (adapter_stream == null) return;
          adapter_stream.send({subPoll: {}});
        },
        // sub_change();
        async (minInterval, maxInterval, timeout) => {
          // if we need a new stream, get one
          if (adapter_response == null) {
            adapter_response = (async () => {
              let req = {
                subChange: {
                  uuid: this.uuid,
                  variable: path,
                  minInterval: minInterval,
                  maxInterval: maxInterval,
                  timeout: timeout
                }
              };
              let [response, s] = await this.ns._conn.requestStream(req);
              checkError(response);
              if (response.payload != 'subType') {
                s.close();
                adapter_stream = null;
                return [null, null];
              }
              // add listener to the stream
              s.received.add((m) => {
                if (m.cancel) adapter.close();
                else if (m.subUpdate && adapter_type) {
                  adapter.update(Value.unpack(m.subUpdate, adapter_type));
                }
              });
              s.start();
              adapter_stream = s;
              adapter_type = Type.unpack(response.subType);
              return [s, adapter_type];
            })();
            await adapter_response;
            if (adapter_stream == null)
              throw new Error('Failed to subscribe!');
          } else {
            // we have a stream, send an update
            await adapter_response;
            let req = {
              subChange: {
                minInterval: minInterval,
                maxInterval: maxInterval,
                timeout: timeout
              }
            };
            if (!adapter_stream) throw new Error("Bad stream!");
            var res = await adapter_stream.request(req);
            if (!res.success) throw new Error("Subscription change failed!");
          }
          // get the type
          let [s, type] = await adapter_response;
          return type;
      }, 
      // cancel()
      async (timeout) => {
        this._adapters.set(key, null);
        if (adapter_stream == null) return;
        adapter_stream.send({cancel: timeout});
        adapter_stream.close();
        adapter_response = null;
        adapter_stream = null;
        adapter_type = null;
      });
      this._adapters.set(key, adapter);
    }
    return await adapter.subscribe(minInterval, maxInterval, timeout);
  }

  async request(params) {
    if (!this.ns || !this.ns._conn) throw new Error("Not connected!");
    var msg = {
      streamContext: {
        uuid: this.uuid,
        params: Params.pack(params)
      }
    }
    var req = new Request();
    var [res, stream] = await this.ns._conn.requestStream(msg);
    stream.received.add((packet) => {
      // parse the packet
      if (packet.streamUpdate) {
        req.process(Params.unpack(packet.streamUpdate));
      } else if (packet.cancel != undefined) {
        stream.close();
        req.close();
      }
    });
    req.closed.add(() => {
      stream.send({cancel:0});
      stream.close();
    });

    try {
      checkError(res);
      if (!res.success) return null;
      // start processing messages in the stream
      stream.start();
    } catch (e) {
      stream.close();
      throw e;
    }
    return req;
  }

  async mount(s) {
    if (!s) throw new Error("Bad source");
    var msg = {
      mount : { src: s.uuid, tgt: this.uuid }
    };
    var res = await this.ns._conn.requestResponse(msg);
    checkError(res);
    if (!res.success) throw new Error("Mount failed!");
  }

  async unmount(s) {
    if (!s) throw new Error("Bad source");
    var msg = {
      unmount : { src: s.uuid, tgt: this.uuid }
    };
    var res = await this.ns._conn.requestResponse(msg);
    checkError(res);
    if (!res.success) throw new Error("Unmount failed!");
  }

  async destroy() {
    await this.ns.destroyContext(this.uuid);
  }
}

class RemoteComponent extends Component {
  constructor(ns, uuid, name, type, params) {
    super();
    this.ns = ns;
    this.uuid = uuid;
    this.name = name;
    this.type = type;
    this.params = params;
  }

  async request(params) {
    if (!this.ns || !this.ns._conn) throw new Error("Not connected!");
    var msg = {
      streamComponent: {
        uuid: this.uuid,
        params: Params.pack(params)
      }
    }
    var req = new Request();
    var [res, stream] = await this.ns._conn.requestStream(msg);
    stream.received.add((packet) => {
      // parse the packet
      if (packet.streamUpdate) {
        req.process(Params.unpack(packet.streamUpdate));
      } else if (packet.cancel != undefined) {
        stream.close();
        req.close();
      }
    });
    req.closed.add(() => {
      stream.send({cancel:0});
      stream.close();
    });

    try {
      checkError(res);
      if (!res.success) return null;
      // start processing messages in the stream
      stream.start();
    } catch (e) {
      stream.close();
      throw e;
    }
    return req;
  }

  async destroy() {
    await this.ns.destroyComponent(this.uuid);
  }
}
