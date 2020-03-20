import { Signal } from './signal.mjs'
import { Container } from './container.mjs'
import { Connection, Params } from './connection.mjs'
import WebSocket from 'isomorphic-ws'

function checkError(packet) {
  if (packet.error) throw new Error(packet.error);
  return res;
}

// A client implements the namespace API
export class Client {
  constructor() {
    this._conn = null;

    this.contexts = new Container();
    this.tasks = new Container();
    this.mounts = new Container();
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
    var nsRes = await this._conn.requestStream({queryNs: {}},
            (packet) => {
        // handle Context/Task/Mount added/removed
        console.log('received queryNs stream packet');
        console.log(packet);
    });
    checkError(nsRes);
    console.log(nsRes);
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
    console.log(res);
  }

  async createTask(name, type, params={}, srcs={}) {
  }
}

class RemoteContext {
}

class RemoteTask {
}

