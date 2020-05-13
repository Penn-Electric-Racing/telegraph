import { Signal } from './signal.mjs'
import { Node } from './nodes.mjs'
import { Collection } from './collection.mjs'
import { Connection, Params } from './connection.mjs'
import WebSocket from 'isomorphic-ws'
import { NamespaceQuery } from './query.mjs'
import { Component, Context, Namespace } from './namespace.mjs'

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
    var nsRes = await this._conn.requestStream({queryNs: {}}, (packet) => {
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
          this.components._removeUUID(packet.contextRemoved);

        } else if (packet.mountAdded) {
          var m = packet.mountAdded;
          this.mounts._add({ uuid : m.src + '/' + m.tgt, src: m.src, tgt: m.tgt });

        } else if (packet.mountRemoved) {
          var u = packet.mountRemoved.src + '/ ' + packet.mountRemoved.tgt;
          this.mounts._removeUUID(u);
        }
    });
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
      this.mounts._add({ uuid : m.src + '/' + m.tgt, src: m.src, tgt: m.tgt });
    }
  }

  async createContext(name, type, params={}, srcs={}) {
    var convertedSrcs = {}
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
    if (res.success == false) return null;
    if (!res.contextCreated) throw new Error("unexpected response: " + JSON.stringify(res));
    return this.contexts.get(res.contextCreated);
  }

  async createComponent(name, type, params={}, srcs={}) {
    var convertedSrcs = {}
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
    if (res.success == false) return null;
    if (!res.componentCreated) throw new Error("unexpected response: " + JSON.stringify(res));
    return this.components.get(res.componentCreated);
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
      var res = await conn.requestResponse(msg);
      checkError(res);
      if (!res.fetchedTree) throw new Error("unexpected response: " + res);
      if (!this._cached_tree) {
        var tree = Node.unpack(res.fetchedTree);
        tree.setContext(this);
        this._cached_tree = tree;
      }
      return this._cached_tree;
    } else {
      return this._cached_tree;
    }
  }

  async subscribe(variable, minInterval, maxInterval, timeout) {
    return await this.subscribePath(variable.path(), minInterval, maxInterval, timeout);
  }

  async subscribePath(path, minInterval, maxInterval, timeout) {
    var key = path.join('/');
    var adapter = this._adapters.get(key);
    if (!adapter) {
      adapter = new Adapter();
      adapter.onClose.add(() => this.adapters_.delete(key));
      this._adapters.set(key, adapter);
    }
  }

  async destroy() {
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
}
