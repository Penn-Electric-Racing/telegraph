import Signal from 'signals'
import uuidv4 from 'uuid'
import WebSocket from 'isomorphic-ws'

// load the generated protobuf file
import api from './api.js'
let { APIPacket } = api.telegraph.proto;

export class Type {
  constructor(ident, name = null, labels = []) {
    this._ident = ident;
    this._name = name;
    this._labels = labels;
  }

  packValue(val) {
    let type = null;
    let value = null;
    switch(this._ident) {
      case Type.NONE._ident:   type = 'none'; value = {none:{}}; break;
      case Type.BOOL._ident:   type = 'bool'; value = {b : val}; break;
      case Type.ENUM._ident:   type = 'en'; value = {en: val}; break;
      case Type.UINT8._ident:  type = 'ui8'; value = { ui8: val}; break;
      case Type.UINT16._ident: type = 'ui16'; value = { ui16: val}; break;
      case Type.UINT32._ident: type = 'ui32'; value = { ui32: val}; break;
      case Type.UINT64._ident: type = 'ui64'; value = { ui64: val}; break;
      case Type.INT8._ident:   type = 'i8'; value = { i8: val}; break;
      case Type.INT16._ident:  type = 'i16'; value = { i16: val}; break;
      case Type.INT32._ident:  type = 'i32'; value = { i32: val}; break;
      case Type.INT64._ident:  type = 'i64'; value = { i64: val}; break;
      case Type.FLOAT ._ident: type = 'f'; value = { f: val}; break;
      case Type.DOUBLE._ident: type = 'd'; value = { d: val}; break;
      default: type = 'empty'; value = {empty:{}}; break;
    }
    return { type: type, ...value };
  }

  unpackValue(proto) {
    var x = undefined;
    switch(this._ident) {
      case Type.NONE._ident:   x = null; break;
      case Type.BOOL._ident:   x = proto.b; break;
      case Type.ENUM._ident:   x = proto.en; break;
      case Type.UINT8._ident:  x = proto.ui8; break;
      case Type.UINT16._ident: x = proto.ui16; break;
      case Type.UINT32._ident: x = proto.ui32; break;
      case Type.UINT64._ident: x = proto.ui64; break;
      case Type.INT8._ident:   x = proto.i8; break;
      case Type.INT16._ident:  x = proto.i16; break;
      case Type.INT32._ident:  x = proto.i32; break;
      case Type.INT64._ident:  x = proto.i64; break;
      case Type.FLOAT ._ident: x = proto.f; break;
      case Type.DOUBLE._ident: x = proto.d; break;
    }
    if (x == undefined)
      throw new Error("Received type does not match expected type");
    return x;
  }

  pack() {
    var type = null;
    switch (this._ident) {
      case Type.BOOL._ident:   type = "BOOL"; break;
      case Type.NONE._ident:   type = "NONE"; break;
      case Type.ENUM._ident:   type = "ENUM"; break;
      case Type.UINT8._ident:  type = "UINT8"; break;
      case Type.UINT16._ident: type = "UINT16"; break;
      case Type.UINT32._ident: type = "UINT32"; break;
      case Type.UINT64._ident: type = "UINT64"; break;
      case Type.INT8._ident:   type = "INT8"; break;
      case Type.INT16._ident:  type = "INT16"; break;
      case Type.INT32._ident:  type = "INT32"; break;
      case Type.INT64._ident:  type = "INT64"; break;
      case Type.FLOAT ._ident: type = "FLOAT"; break;
      case Type.DOUBLE._ident: type = "DOUBLE"; break;
      default: type = "INVALID"; break;
    }
    return {name: this._name, type: type, labels: this._labels};
  }

  toString() {
    switch (this._ident) {
      case Type.BOOL._ident:   return 'bool';
      case Type.NONE._ident:   return 'none';
      case Type.ENUM._ident:   return 'enum [' + this._labels.join(', ') + ']';
      case Type.UINT8._ident:  return 'uint8';
      case Type.UINT16._ident: return 'uint16';
      case Type.UINT32._ident: return 'uint32';
      case Type.UINT64._ident: return 'uint64';
      case Type.INT8._ident:   return 'int8';
      case Type.INT16._ident:  return 'int16';
      case Type.INT32._ident:  return 'int32';
      case Type.INT64._ident:  return 'int64';
      case Type.FLOAT._ident:  return 'float';
      case Type.DOUBLE._ident: return 'double';
      default: return 'invalid';
    }
  }
}

// numbers are used for the switch statements
Type.INVALID = new Type(0);
Type.NONE = new Type(1);
Type.ENUM = new Type(2);
Type.BOOL = new Type(3);
Type.UINT8 = new Type(4);
Type.UINT16 = new Type(5);
Type.UINT32 = new Type(6);
Type.UINT64 = new Type(7);
Type.INT8 = new Type(8);
Type.INT16 = new Type(9);
Type.INT32 = new Type(10);
Type.INT64 = new Type(11);
Type.FLOAT = new Type(12);
Type.DOUBLE = new Type(13);

Type.unpack = function(proto) {
  switch(proto.type) {
    case "INVALID": return Type.INVALID;
    case "NONE": return Type.NONE;
    case "BOOL": return Type.BOOL;
    case "UINT8": return Type.UINT8;
    case "UINT16": return Type.UINT16;
    case "UINT32": return Type.UINT32;
    case "UINT64": return Type.UINT64;
    case "INT8": return Type.INT8;
    case "INT16": return Type.INT16;
    case "INT32": return Type.INT32;
    case "INT64": return Type.INT64;
    case "FLOAT": return Type.FLOAT;
    case "DOUBLE": return Type.DOUBLE;
    case "ENUM": return new Type(Type.ENUM._ident, proto.name, proto.labels);
    default: return Type.INVALID;
  }
};

export class Subscription {
  constructor(type, minInterval, maxInterval) {
    this._type = type;
    this._minInteval = minInterval;
    this._maxInterval = maxInterval;

    this.onData = new Signal();
    this.onCancel = new Signal();
  }

  getType() { return this._type; }
  getMinInterval() { return this._minInterval; }
  getMaxInterval() { return this._maxInterval; }

  data(ts, val) {
    this.onData.dispatch(ts, val);
  }

  addHandler(h) {
    this.onData.add(h);
  }

  removeHandler(h) {
    this.onData.remove(h);
  }

  cancel() {
    this.onCancel.dispatch();
  }
};


// the equivalent of the 'nodes' directory in the c++ version
export class Node {
  constructor(ctx, parent, name, pretty, desc) {
    this._ctx = ctx;
    this._parent = parent;
    this._name = name;
    this._pretty = pretty;
    this._desc = desc;
  }

  getName() { return this._name; }
  getPretty() { return this._pretty; }
  getDesc() { return this._desc; }
  getParent() { return this._parent; }
}

export class Group extends Node {
  constructor(ctx, parent, name, pretty, desc, 
              schema, version, children) {
    super(ctx, parent, name, pretty, desc);
    this._schema = schema;
    this._version = version;
    this._children = children;
    this._childrenMap = new Map(); // name to child map
    for (let c of this._children) {
      this._childrenMap.set(c.getName(), c);
    }
  }

  getSchema() { return this._schema; }
  getVersion() { return this._version; }
  getChildren() { return this._children; }

  child(child_name) {
    var c = this._childrenMap.get(child_name);
    return c;
  }

  toString(indent = 0) {
    let line = ' '.repeat(indent) + 
        this.getName() + ' ' + this.getSchema() + '/' + this.getVersion() + 
          ' (' + this.getPretty() + '): ' + this.getDesc();

    // add the children
    if (this._children.length > 0) 
      line += '\n' + this._children.map(c => c.toString(indent + 4)).join('\n');

    return line;
  }
}

export class Variable extends Node {
  constructor(name, pretty, desc, type) {
    super(name, pretty, desc);
    this._type = type;
    this._subscriptions = [];

    this.onSubscribe = new Signal();
  }

  getType() { return this._type; }

  dispose() {
    for (let s of this._subscriptions) {
      s.cancel();
    }
    this.onDispose.dispatch();
  }

  subscribe(minInterval, maxInterval) {
    let s = new Subscription(this._type, minInterval, maxInterval);
    this._subscriptions.push(s);
    s.onCancel.add(() => {
      let i = this._subscriptions.indexOf(s);
      if (i >= 0) this._subscriptions.splice(i, 1);
    });
    this.onSubscribe.dispatch(s);
    return s;
  }

  toString(indent = 0) {
    let line = ' '.repeat(indent) + 
        this.getName() + ' ' + this.getType() + 
          ' (' + this.getPretty() + '): ' + this.getDesc();
    return line;
  }
}

export class Action extends Node {
  constructor(name, pretty, desc, argType, retType) {
    super(name, pretty, desc);
    this._argType = argType;
    this._retType = retType;
    this._actor = null;
  }

  setActor(actor) {
    this._actor = actor;
  }

  async call(arg) {
    if (!this._actor) return null;
    return await this._actor(arg);
  }

  getArgType() { return this._argType; }
  getRetType() { return this._retType; }

  toString(indent = 0) {
    let line = ' '.repeat(indent) + 
          this.getName() + ' ' + this.getArgType() + ' -> ' + this.getRetType() +
          ' (' + this.getPretty() + '): ' + this.getDesc();
    return line;
  }
}

export class Stream extends Node {
  constructor(name, pretty, desc) {
    super(name, pretty, desc);
  }

  toString(indent = 0) {
    let line = ' '.repeat(indent) + 
        this.getName() + ' stream (' + this.getPretty() + '): ' + this.getDesc();
    return line;
  }
}

export class Tree {
  constructor(root) {
    if (!root) throw new Error("Root of a tree cannot be null");

    this.root = root;
    this.onNodeAdded = new Signal();
    this.onNodeRemoved = new Signal();
    this.root.onDescendantAdded.add(this.onNodeAdded.dispatch);
    this.root.onDescendantRemoved.add(this.onNodeRemoved.dispatch);
  }

  getRoot() {
    return this.root
  }

  *nodes() {
    yield this.root;
    for (let n of this.root.descendants()) {
      yield n;
    }
  }

  find(path) {
    return this.root.find(path);
  }

  dispose() {
    this.root.dispose();
  }

  inspect() {
    return this.toString();
  }

  toString() {
    return this.root.toString();
  }
}

export class Feed {
  constructor(initial) {
    this.all = initial;
    this.added = new Signal();
    this.removed = new Signal();
    this.closed = new Signal();
  }

  close() {
    this.closed.dispatch(this);
  }
}

// a special kind of feed specially
// for data queries
export class DataFeed {

}

export class Context {
  // none of these things can change 
  // during the lifetime of the context!
  constructor(ns, uuid, name, type, info) {
    this._ns = ns;
    this._uuid = uuid;
    this._name = name;

    this._type = type; // 'device', 'archive', or 'container'
    this._info = info; // type specific context information
  }

  getName() { return this._name; }
  getUUID() { return this._uuid; }
  getType() { return this._type; }
  getInfo() { return this._info; }

  async fetch() {
    return await this._ns.fetch(this._uuid);
  }

  // returns a feed object with mounts
  async mounts(as_src=true, as_tgt=true) {
    return await this._ns.mounts(as_src ? this.getUUID() : null, 
                                 as_tgt ? this.getUUID() : null);
  }

  toString() {
    return this._name;
  }
}

class Task {

}

// a namespace can handle all of the operations
// defined in api.proto. A relay then exposes a namespace.
export class Namespace {
  constructor(uuid) { this._uuid = uuid; }

  getUUID() { return this._uuid; }

  // queries with options
  async mounts(options) {  return null; }
  async tasks(options) { return null; }
  async contexts(options) { return null; }

  // variable is either { ctx : uuid, path: } or a node object
  async data(location) { return null; }
  async writeData(location, data) { return null; }

  // variable is either { ctx : uuid, path: } or a variable object
  async subscribe(variable, min_interval, max_interval) { return null; }

  // action is either { ctx : uuid, path: } or an action object
  async act(action, arg) { return null; }
}

export class LocalNamespace extends Namespace {
  constructor(uuid) {
    if (!uuid) uuid = uuidv4()
    super(uuid)
  }
}

// Server/client stuff....

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
        var c = new Connection(ws);
        var r = new RemoteHandler(c, this._ns);

        // need to setup the remote handler
        // to make sure it has uuid 
        // and the handlers setup
        await r.init();

        if (!r.getUUID()) throw new Error('Remote UUID not initialized!');
        this._register(r);
        this._clients.push(r);
      })();
    });
  }

  async connect(address) {
    var client = new WebSocket(address);

    await new Promise(function(resolve, reject) {
      client.on('open', resolve);
    });

    var c = new Connection(client);
    var r = new RemoteHandler(c, this._ns);
    await r.init();

    if (!r.getUUID()) throw new Error('Remote UUID not initialized!');
    this._register(r);
    this._connections.push(r);
  }

  terminate() {
  }
}

// handles the low-level serialization
// and packet-id system
export class Connection {
  constructor(ws) {
    this._ws = ws;
    this._proto = proto;
    this._req_num;
  }

  send(payload) {
  }
}

class RemoteHandler extends Namespace {
  // the remote relay's uuid, the conneciton
  // we use to interface with it, and the local namespace
  constructor(conn, local) {
    super(null);
    this._conn = conn;
    this._local = local;
  }

  // needs to be separate from constructor so we can use async
  async init() {
    console.log(msg);
  }
}
