import { Signal } from './signal.mjs'
import { Node } from './nodes.mjs'
import { Collection } from './collection.mjs'
import { Connection, Params } from './connection.mjs'
import WebSocket from 'isomorphic-ws'

function checkError(packet) {
  if (packet.error) throw new Error(packet.error);
  return packet;
}

// A client implements the namespace API
export class Client {
  constructor() {
    this._conn = null;

    this.contexts = new Collection();
    this.tasks = new Collection();
    this.mounts = new Collection();
  }

  async connect(address) {
    if (this._conn) throw new Error('Already connected!');
    try {
      this._conn = new Connection(new WebSocket(address), true);
      this._conn.onClose.add(() => {
        this.contexts.clear();
        this.tasks.clear();
        this.mounts.clear();
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

  query() {
    return new ClientQuery(this);
  }

  async _queryNS() {
    // send a queryNs
    var nsRes = await this._conn.requestStream({queryNs: {}}, (packet) => {
        // handle Context/Task/Mount added/removed
        if (packet.contextAdded) {
          let c = packet.contextAdded;
          this.contexts.add(new RemoteContext(this, c.uuid, c.name, c.type, Params.unpack(c.params)));

        } else if (packet.contextRemoved) {
          this.contexts.removeUUID(packet.contextRemoved);

        } else if (packet.taskAdded) {
          let t = packet.taskAdded;
          this.tasks.add(new RemoteTask(this, t.uuid, t.name, t.type, Params.unpack(t.params)));

        } else if (packet.taskRemoved) {
          this.tasks.removeUUID(packet.contextRemoved);

        } else if (packet.mountAdded) {
          var m = packet.mountAdded;
          this.mounts.add({ uuid : m.src + '/' + m.tgt, src: m.src, tgt: m.tgt });

        } else if (packet.mountRemoved) {
          var u = packet.mountRemoved.src + '/ ' + packet.mountRemoved.tgt;
          this.mounts.removeUUID(u);
        }
    });
    checkError(nsRes);
    // populate the namespace based on the query
    for (let c of nsRes.ns.contexts) {
      this.contexts.add(new RemoteContext(this, c.uuid, c.name, c.type, Params.unpack(c.params)));
    }
    for (let t of nsRes.ns.tasks) {
      this.tasks.add(new RemoteTask(this, t.uuid, t.name, t.type, Params.unpack(t.params)));
    }
    for (let m of nsRes.ns.mounts) {
      this.mounts.add({ uuid : m.src + '/' + m.tgt, src: m.src, tgt: m.tgt });
    }
  }

  async createContext(name, type, params={}, srcs={}) {
    var convertedSrcs = {}
    var msg = {
      createContext : {
        name: name,
        type: type,
        params: Params.pack(params),
        srcs: convertedSrcs
      }
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.succes == false) return null;
    if (!res.contextCreated) throw new Error("unexpected response: " + res);
    return this.contexts.get(res.contextCreated);
  }

  async createTask(name, type, params={}, srcs={}) {
    var convertedSrcs = {}
    var msg = {
      createTask: {
        name: name,
        type: type,
        params: Params.pack(params),
        srcs: convertedSrcs
      }
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.succes == false) return null;
    if (!res.taskCreated) throw new Error("unexpected response: " + res);
    return this.tasks.get(res.taskCreated);
  }
}

class RemoteContext {
  constructor(ns, uuid, name, type, params) {
    this.ns = ns;
    this.uuid = uuid;
    this.name = name;
    this.type = type;
    this.params = params;

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
      var tree = Node.unpack(res.fetchedTree);
      tree.setContext(this);
      this._cached_tree = tree;
      return tree;
    } else {
      return this._cached_tree;
    }
  }

  async destroy() {
  }
}

class RemoteTask {
  constructor(ns, uuid, name, type, params) {
    this.ns = ns;
    this.uuid = uuid;
    this.name = name;
    this.type = type;
    this.params = params;
  }
}
