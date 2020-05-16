import { Signal } from './signal.mjs'
import WebSocket from 'isomorphic-ws'


import api from '../api.js'
let { Packet } = api.telegraph.api;

export class Params {
  static pack(json) {
    if (json == null || json == undefined) return {none: {}};
    var type = typeof json;
    if (Array.isArray(json)) {
      return { array : { elements : json.map(x => Params.pack(x)) } };
    } else if (type == "object") {
      return { object : { 
        entries : Object.entries(json).map(([k, v]) => 
          { return { key: k, value: Params.pack(v) }; } )
      } }
    } else if (type == "number") {
      return { number: json };
    } else if (type == "string") {
      return { str: json };
    } else if (type == "boolean") {
      return { b : json };
    }
  }
  static unpack(proto) {
    if (proto == null) return null;
    if (proto.array) {
      return protobuf.array.elements.map(Params.unpack);
    }
    if (proto.object) {
      var obj = {};
      for (let {key, value} of proto.object.entries) {
        obj[key] = Params.unpack(value);
      }
    }
    if (proto.number) return proto.number;
    if (proto.str) return proto.str;
    if (proto.b) return proto.b;
    return null;
  }
}

export class Stream {
  constructor(reqId, conn) {
    this.reqId = reqId;
    this.conn = conn;
    this.closed = new Signal();
    this.isClosed = false;
    this.received = new Signal();

    this.buffer = [];
  }
  start() {
    var b = this.buffer;
    this.buffer = null;
    for (let m of b) {
      this.received.dispatch(m);
    }
  }
  process(m) {
    if (this.buffer) {
      this.buffer.push(m);
    } else {
      this.received.dispatch(m);
    }
  }
  close() {
    if (!this.isClosed) {
      this.isClosed = true;
      this.closed.dispatch();
    }
  }
  send(msg) {
    this.conn.writeBack(this.reqId, msg);
  }
  request(msg) {
  }
}

export class Connection {
  constructor(ws, countUp) {
    this._counter = 0;
    this._countUp = countUp;
    this._ws = ws;
    this._ws.binaryType = 'arraybuffer';
    this._ws.onclose = () => { 
      this._handlers.clear();
      // send back a null response to indicate connection closed
      for (let r of this._openRequests.values()) { r(null); }
      this._openRequests.clear();
      this._openStreams.clear();
      this.onClose.dispatch()
    };
    this._ws.onmessage = (msg, flags) => {
      var array = new Uint8Array(msg.data);
      var packet = Packet.decode(array);
      this.received(packet);
    }

    this._handlers = new Map();
    this._openRequests = new Map();
    this._openStreams = new Map();

    this.onClose = new Signal();
  }

  isOpen() { return this._ws.readyState == WebSocket.OPEN; }

  async connect() {
    var timeout = new Promise((res, rej) => setTimeout(res, 1000));
    var open = new Promise((res, rej) => { this._ws.onopen = res; this._ws.onerror = res; });
    await Promise.race([timeout, open]);
    if (!this.isOpen()) throw new Error('Failed to connect');
  }

  disconnect() {
    return new Promise((res, rej) => {
      if (this.isOpen()) {
        this.onClose.add(res)
        this._ws.close();
      } else {
        res();
      }
    });
  }

  setHandler(payloadType, h) {
    this._handlers.set(payloadType, h);
  }

  setStreamCB(reqId, h) {
    this._openStreams.set(reqId, h);
  }

  closeStream(reqId) {
    this._openStreams.delete(reqId);
  }

  writeBack(reqId, payload) {
    payload.reqId = reqId;
    this.send(payload);
  }

  send(packet) {
    console.log('sent:', packet);
    var buffer = Packet.encode(packet).finish();
    this._ws.send(buffer);
  }

  received(packet) {
    console.log('received:', packet);
    var reqId = packet.reqId;
    var payloadType = packet.payload;

    if (this._handlers.has(payloadType)) {
      this._handlers.get(payloadType)(packet);
    }
    if (this._openRequests.has(reqId)) {
      this._openRequests.get(reqId)(packet);
      this._openRequests.delete(reqId);
      return;
    }
    if (this._openStreams.has(reqId)) {
      this._openStreams.get(reqId).process(packet);
    }
  }

  async requestResponse(req) {
    var send = new Promise((res, rej) => {
      var reqId = this._countUp ? this._counter++ : this._counter--;
      this._openRequests.set(reqId, (packet) => packet == null ? rej('Connection closed') : res(packet));
      req.reqId = reqId;
      this.send(req);
    });
    return await send;
  }

  async requestStream(req) {
    var stream = new Stream(-1, this);
    var send = new Promise((res, rej) => {
      var reqId = this._countUp ? this._counter++ : this._counter--;
      stream.reqId = reqId;
      stream.closed.add(() => this._openStreams.delete(reqId));
      this._openRequests.set(reqId, (packet) => packet == null ? rej('Connection closed') : res(packet));
      this._openStreams.set(reqId, stream);
      req.reqId = reqId;
      this.send(req);
    });
    var reply = await send;
    return [reply, stream];
  }
}
