import WebSocket from 'isomorphic-ws'
import Signal from 'signals'

import api from '../api.js'
let { Packet } = api.telegraph.api;

import { Node, Value, Type } from './nodes.mjs'
import { Info, Namespace, Feed, Context } from './namespace.mjs'
import { Adapter } from './adapter.mjs'

export class Relay {
  // the namespace for the relay 
  // and the loaded protobuffers to use for encoding
  constructor(ns, proto) {
    if (!ns) throw new Error('Cannot supply null namespace to a relay');
    this._proto = proto;
    this._ns = ns;
    this._namespaces = new Map();
    this._register(ns);

    // server sockets
    this._server = null;
    this._clients = new Set();

    this._connections = new Set();
  }

  _register(ns) {
    this._namespaces.set(ns.getUUID(), ns);
  }
  _unregister(ns) {
    this._namespaces.delete(ns.getUUID());
  }
  
  bind(port) {
    if (this._server) throw new Error('Server already running!');

    this._server = new WebSocket.Server({ port: port });
    this._clients.clear();

    this._server.on('connection', (ws) => {
      (async () => {
        var c = new Connection(ws, false);
        var r = new RemoteHandler(c, this._namespaces, this._ns);

        // need to setup the remote handler
        // to make sure it has uuid 
        // and the handlers setup
        await r.init();

        if (!r.getUUID()) throw new Error('Remote UUID not initialized!');
        this._register(r);
        this._clients.add(r);
        r.destroyed.add(() => { this._unregister(r); this._clients.delete(r); });
      })();
    });
  }

  async connect(address) {
    var client = new WebSocket(address);

    var timeout = new Promise((res, rej) => setTimeout(res, 1000));
    var open = new Promise(function(resolve, reject) {
      client.onopen = resolve;
      client.onerror = resolve;
    });
    await Promise.race([timeout, open]);
    if (client.readyState != 1) return null;

    var c = new Connection(client, true);
    var r = new RemoteHandler(c, this._namespaces, this._ns);
    await r.init();

    if (!r.getUUID()) throw new Error('Remote UUID not initialized!');
    this._register(r);
    this._connections.add(r);
    r.destroyed.add(() => { 
      this._unregister(r); 
      this._connections.delete(r); 
    });

    return r;
  }

  terminate() {
  }
}

// handles the low-level serialization
// and packet-id system
class Connection {
  constructor(ws, countUp) {
    this.closed = new Signal();

    this._ws = ws;
    this._reqNum = 0;
    this._countUp = countUp;

    this._listeners = new Map();
    this._handlers = new Map();

    this._ws.binaryType = 'arraybuffer';
    this._ws.onclose = () => { this._ws = null; this.closed.dispatch() };
    this._ws.onmessage = (msg, flags) => {
      (async () => {
        var array = new Uint8Array(msg.data);
        var packet = Packet.decode(array);
        var id = packet.reqId;
        var payloadType = packet.payload;

        var done = () => { this._listeners.delete(id); }
        var respond = (res) => {
          if (!this._ws) return;
          // from the response extract the key
          res.reqId = id;
          var buffer = Packet.encode(res).finish();
          this._ws.send(buffer);
        }

        // check if we are listening on this id
        if (this._listeners.has(id)) {
          var l = this._listeners.get(id);
          await l(packet, respond, done);
          return;
        }

        // check if this is a request
        if (this._handlers.has(payloadType)) {
          var h = this._handlers.get(payloadType);

          // call the handler with the packet and a
          // callback to send bakc responses
          var l = await h(packet, respond, done);

          // if it returned a listener, add the listener
          // under the packet's id
          if (l) this._listeners.set(id, l);
        }
      })();
    };
  }

  setHandler(type, h) {
    this._handlers.set(type, h)
  }

  send(payload, listener) {
    if (!this._ws) return;
    if (payload.reqId == undefined) {
      payload.reqId = this._countUp ? ++this._reqNum : --this._reqNum;
    }

    var buffer = Packet.encode(payload).finish();
    if (listener) this._listeners.set(payload.reqId, listener);
    this._ws.send(buffer);
  }

  req(payload) {
    return new Promise((accept, reject) => {
      this.send(payload, function(msg, cancel) {
        accept(msg);
      });
    });
  }

  // stream requests
  // get a reply and 
  // then a string of updates
  reqStream(payload) {
    return new Promise((accept, reject) => {
      var first = true;
      var updateSignal = new Signal();
      this.send(payload, function(msg, respond, done) {
        if (first) {
          accept({
            reply: msg,
            respond: respond,
            update: updateSignal,
            done: done 
          });
          first = false
        } else {
          updateSignal.dispatch(msg);
        }
      });
    });
  }
}

class RemoteSubscription {
  constructor(conn, minInterval, maxInterval) {
    this._conn = conn;
    this._publisher = new Publisher(false);
  }
}

class RemoteHandler extends Namespace {
  // the remote relay's uuid, the conneciton
  // we use to interface with it, and the local namespace
  constructor(conn, namespaces, local) {
    super(null);
    this._conn = conn;
    this._conn.closed.add(() => { this.destroy() });
    this._namespaces = namespaces;
    this._local = local;

    // any local feeds we have open
    // to process inbound queries
    this._localFeeds = new Set();
    // local subscriptions 
    // we have open
    this._localSubs = new Set();

    // subscription adapters
    // for the remote namespace
    this._subAdapters = new Map();
  }

  destroy() {
    this._conn = null;
    for (let o of this._localFeeds) {
      o.close();
    }
    this.destroyed.dispatch();
  }

  // needs to be separate from constructor so we can use async
  async init() {
    this._conn.setHandler('queryNs', (m, reply) => {
      reply({nsUuid: this._local.getUUID()});
    });
    this._conn.setHandler('changeSub', (m, r) => this._handleSubscribe(m, r));
    this._conn.setHandler('contextsQuery', (m, r) => this._handleContexts(m, r));
    this._conn.setHandler('fetchTree', (m, r) => this._handleFetch(m, r));

    // query for the remote's namespace uuid
    var msg = { queryNs : {} }
    var uuidRes = await this._conn.req(msg);
    this._uuid = uuidRes.nsUuid;
  }

  async _handleContexts(m, reply) {
    var query = m.contextsQuery;
    var f = await this._local.contexts({by_uuid:query.uuid, by_name:query.name, by_type:query.type});
    // send back all the items in the feed
    reply({contextList: { contexts: [...f.all].map(ctx => ctx.pack()) } });

    var al = (c) => { reply({contextAdded: c.pack() }); };
    var rl = (c) => { reply({contextRemoved: c.pack() }); };
    f.added.add(al);
    f.removed.add(rl);
    this._localFeeds.add(f);
    // on cancel, stop the feed
    return (m) => {
      if (m.cancel) {
        this._localFeeds.delete(f);
        f.close();
      }
    };
  }

  async contexts({by_uuid=null, by_name=null, by_type=null}) {
    var query = { contextsQuery : {uuid:by_uuid, name:by_name, type:by_type} };
    var stream = await this._conn.reqStream(query);
    var ctxList = stream.reply;

    var contexts = new Set();
    for (let c of ctxList.contextList.contexts) {
      contexts.add(RemoteContext.unpack(this._namespaces, c));
    }
    var f = new Feed(contexts);
    f.closed.add(() => {
      // stop getting updates from
      // the stream
      stream.done();
      stream.respond({cancel:{}});
    });
    return f;
  }

  async _handleFetch(m, respond) {
    var uuid = m.fetchTree;
    var tree = await this._local.fetch(uuid);
    respond({fetchedTree: tree.pack()});
  }

  async fetch(uuid, ctx=null) {
    var msg = { fetchTree: uuid };
    var res = await this._conn.req(msg);
    var root = Node.unpack(res.fetchedTree);
    root.setContext(ctx);
    return root;
  }

  async _handleSubscribe(m, respond) {
      var sub = m.changeSub;
      var s = await this._local.subscribe(sub.uuid, sub.variable, 
                                          sub.minInterval, sub.maxInterval);
      var r = {subType: s != null ? s.type.pack() : Type.INVALID.pack() };
      respond(r);
      if (s) {
        s.data.add(v => {
          respond({variableUpdate: Value.pack(v, s.type)})
        });
        return (m, res, done) => {
          if (m.cancel) {
            s.cancel();
            done();
          }
        };
      }
  }

  async subscribe(ctxUuid, path, minInterval, maxInterval) {
    var key = ctxUuid + '/' + path.join('/');
    var adapter = this._subAdapters.get(key);
    if (!adapter) {
      // send the request
      var req = { changeSub: {
          uuid: ctxUuid, variable: path, 
          minInterval: minInterval, maxInterval: maxInterval
      }};

      var stream = await this._conn.reqStream(req);
      if (!stream.reply.subType) {
        stream.done();
        return null;
      }
      var type = Type.unpack(stream.reply.subType);
      if (!type.valid) {
        stream.done();
        return null;
      }

      stream.update.add(m => {
        if (m.variableUpdate) adapter.update(Value.unpack(m.variableUpdate))
      });
      this._subAdapters.set(key, adapter);

      var firstSub = true;
      adapter = new Adapter(type,
        async (minInterval, maxInterval) => {
          if (!firstSub) {
            var req = { changeSub: {
                uuid: ctxUuid, variable: path, 
                minInterval: minInterval, maxInterval: maxInterval
            }};
            stream.respond(req);
          }
          firstSub = false;
        }, async () => {
          if (stream) {
            stream.done();
            stream.respond({cancel:{}});
            stream = null;
            // remove the adapter
            this._subAdapters.delete(key);
          }
        }
      );
    }
    return await adapter.subscribe(minInterval, maxInterval);
  }
}

// the remote context just forwards all the requests to the remote handler
class RemoteContext extends Context {
  constructor(ns, uuid, name, type, info) {
    super(ns, uuid, name, type, info);
  }

  fetch() {
    return this._ns.fetch(this.getUUID(), this);
  }

  mounts(as_src=true, as_tgt=true) {
    return this._ns.mounts(as_src ? this.getUUID() : null, 
                           as_tgt ? this.getUUID() : null);
  }

  mount(src) { return this._ns.mount(src, this); }
  unmount(src) { return this._ns.umount(src, this); }

  subscribe(variable, minInterval, maxInterval) { 
    return this._ns.subscribe(this.getUUID(), 
      variable.path(), minInterval, maxInterval);
  }
  call(action, arg) { return this._ns.call(this.getUUID(), action.path(), arg); }

  writeData(node, data) { return this._ns.writeData(this.getUUID(), node.path(), data); }
  queryData(node) { return this._ns.queryData(this.getUUID(), node.path()); }

  destroy() { return this._ns.destroyContext(this.getUUID()); }

  static unpack(namespaces, protobuf) {
    var ns = namespaces.get(protobuf.ns);
    if (!ns) throw new Error("Unable to find namespace!");
    return new RemoteContext(ns, protobuf.uuid, protobuf.name, protobuf.type, 
                              Info.unpack(protobuf.info));
  }
}
