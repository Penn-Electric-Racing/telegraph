import { Signal } from './signal.mjs'
import WebSocket from 'isomorphic-ws'


import api from '../api.js'
let { Packet } = api.telegraph.api;

export class Params {
  static pack(json) {
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
    if (protobuf.array) {
      return protobuf.array.elements.map(Params.unpack);
    }
    if (protobuf.object) {
      var obj = {};
      for (let {key, value} of protobuf.object.entries) {
        obj[key] = Params.unpack(value);
      }
    }
    if (protobuf.number) return protobuf.number;
    if (protobuf.str) return protobuf.str;
    if (protobuf.b) return protobuf.b;
    return null;
  }
}

export class Connection {
  constructor(ws, countUp) {
    this._counter = 0;
    this._countUp = countUp;
    this._ws = ws;
    this._ws.onclose = () => { this.onClose.dispatch() };
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
    var buffer = Packet.encode(packet).finish();
    this._ws.send(buffer);
  }

  received(packet) {
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
      this._openStreams.get(reqId)(packet);
    }
  }

  async requestResponse(req) {
    var send = new Promise((res, rej) => {
      var reqId = this._countUp ? this._counter++ : this._counter--;
      this._openRequests.set(reqId, (packet) => res(packet));
      req.reqId = reqId;
      this.send(req);
    });
    return await send;
  }

  async requestStream(req, handler) {
    var send = new Promise((res, rej) => {
      var reqId = this._countUp ? this._counter++ : this._counter--;
      this._openRequests.set(reqId, (packet) => res(packet));
      this._openStreams.set(reqId, handler);
      req.reqId = reqId;
      this.send(req);
    });
    return await send;
  }
}
