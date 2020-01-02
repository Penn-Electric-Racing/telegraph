import WebSocket from 'isomorphic-ws'
import Signal from 'signals'

import api from '../api.js'
let { APIPacket } = api.telegraph.proto;

import { Info, Node, 
  Namespace, Feed, Context } from './namespace.mjs'

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
    this._namespaces.remove(ns.getUUID());
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
      })();
    });
  }

  async connect(address) {
    var client = new WebSocket(address);

    await new Promise(function(resolve, reject) {
      client.on('open', resolve);
    });

    var c = new Connection(client, true);
    var r = new RemoteHandler(c, this._namespaces, this._ns);
    await r.init();

    if (!r.getUUID()) throw new Error('Remote UUID not initialized!');
    this._register(r);
    this._connections.add(r);

    return r;
  }

  terminate() {
  }
}

// handles the low-level serialization
// and packet-id system
class Connection {
  constructor(ws, countUp) {
    this._ws = ws;
    this._reqNum = 0;
    this._countUp = countUp;

    this._listeners = new Map();
    this._handlers = new Map();

    this._ws.on('message', (data, flags) => {
      (async () => {
        var packet = APIPacket.decode(data);
        var id = packet.reqId;
        var payloadType = packet.payload;

        // check if we are listening on this id
        if (this._listeners.has(id)) {
          var l = this._listeners.get(id);
          await l(packet, () => { this._listeners.remove(id); });
          return;
        }

        // check if this is a request
        if (this._handlers.has(payloadType)) {
          var h = this._handlers.get(payloadType);

          // call the handler with the packet and a
          // callback to send bakc responses
          var l = await h(packet, (res) => {
            // from the response extract the key
            res.reqId = id;
            var buffer = APIPacket.encode(res).finish();
            this._ws.send(buffer);
          });

          // if it returned a listener, add the listener
          // under the packet's id
          if (l) this._listeners.set(id, l);
        }
      })();
    });
  }

  setHandler(type, h) {
    this._handlers.set(type, h)
  }

  send(payload, listener) {
    if (payload.reqId == undefined) {
      payload.reqId = this._countUp ? ++this._reqNum : --this._reqNum;
    }

    var buffer = APIPacket.encode(payload).finish();
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
      this.send(payload, function(msg, stop) {
        if (first) {
          accept({
            res: msg,
            update: updateSignal,
            cancel: stop
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
    this._namespaces = namespaces;
    this._local = local;

    this._subAdapters = new Map();
  }

  // needs to be separate from constructor so we can use async
  async init() {
    this._conn.setHandler('queryNs', (m, reply) => {
      reply({nsUuid: this._local.getUUID()});
    });

    this._conn.setHandler('contextsQuery', async (m, reply) => {
      var query = m.contextsQuery;
      var f = await this._local.contexts({by_uuid:query.uuid, by_name:query.name, by_type:query.type});
      // send back all the items in the feed
      reply({contextList: { contexts: [...f.all].map(ctx => ctx.pack()) } });

      var al = (c) => { reply({contextAdded: c.pack() }); };
      var rl = (c) => { reply({contextRemoved: c.pack() }); };
      f.added.add(al);
      f.removed.add(rl);
      // on cancel, stop the feed
      return (m) => {
        if (m.cancel) f.cancel();
      };
    });

    this._conn.setHandler('fetchTree', async (m, reply) => {
      var uuid = m.fetchTree;
      var tree = await this._local.fetch(uuid);
      reply({fetchedTree: tree.pack()});
    });

    // query for the remote's namespace uuid
    var msg = { queryNs : {} }
    var uuidRes = await this._conn.req(msg);
    this._uuid = uuidRes.nsUuid;
  }

  async contexts({by_uuid=null, by_name=null, by_type=null}) {
    var query = { contextsQuery : {uuid:by_uuid, name:by_name, type:by_type} };
    var stream = await this._conn.reqStream(query);
    var ctxList = stream.res;

    var contexts = new Set();
    for (let c of ctxList.contextList.contexts) {
      contexts.add(RemoteContext.unpack(this._namespaces, c));
    }
    var f = new Feed(contexts);
    return f;
  }

  async fetch(uuid) {
    var msg = { fetchTree: uuid };
    var res = await this._conn.req(msg);
    var root = Node.unpack(res.fetchedTree);
    root.setContext(this);
    return root;
  }

  async subscribe(ctxUuid, path, minInterval, maxInterval) {
    var key = ctxUuid + '/' + path.join('/');
    var adapter = this._subAdapters.get(key);
    if (!adapter) {
      adapter = new Adapter(() => {
      }, () => {
        // remove the adapter, cancel the last request
        this._subAdapters.remove(key);
      });
      this._subAdapters.set(key, adapter);
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
    return this._ns.fetch(this.getUUID());
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
