import Signal from 'signals'

export class Type {
  static INVALID = new Type('invalid', [], 0);
  static NONE = new Type('none', [], 1);
  static ENUM = new Type('enum', [], 2);
  static BOOL = new Type('bool', [], 3);
  static UINT8 = new Type('uint8', [], 4);
  static UINT16 = new Type('uint16', [], 5);
  static UINT32 = new Type('uint32', [], 6);
  static UINT64 = new Type('uint64', [], 7);
  static INT8 = new Type('int8', [], 8);
  static INT16 = new Type('int16', [], 9);
  static INT32 = new Type('int32', [], 10);
  static INT64 = new Type('int64', [], 11);
  static FLOAT = new Type('float', [], 12);
  static DOUBLE = new Type('double', [], 13);

  constructor(name = null, labels = [], class_=2) {
    this._class = class_;
    this._name = name;
    this._labels = labels;
  }

  pack() {
    var type = null;
    switch (this._ident) {
      case Type.BOOL._class:   type = "BOOL"; break;
      case Type.NONE._class:   type = "NONE"; break;
      case Type.ENUM._class:   type = "ENUM"; break;
      case Type.UINT8._class:  type = "UINT8"; break;
      case Type.UINT16._class: type = "UINT16"; break;
      case Type.UINT32._class: type = "UINT32"; break;
      case Type.UINT64._class: type = "UINT64"; break;
      case Type.INT8._class:   type = "INT8"; break;
      case Type.INT16._class:  type = "INT16"; break;
      case Type.INT32._class:  type = "INT32"; break;
      case Type.INT64._class:  type = "INT64"; break;
      case Type.FLOAT ._class: type = "FLOAT"; break;
      case Type.DOUBLE._class: type = "DOUBLE"; break;
      default: type = "INVALID"; break;
    }
    return {name: this._name, type: type, labels: this._labels};
  }

  static unpack(proto) {
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
      case "ENUM": return new Type(proto.name, proto.labels, Type.ENUM._class);
      default: return Type.INVALID;
    }
  }

  toString() {
    switch (this._ident) {
      case Type.BOOL._ident:   return 'bool';
      case Type.NONE._ident:   return 'none';
      case Type.ENUM._ident:   return 'enum ' + this._name + ' [' + this._labels.join(', ') + ']';
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



// the equivalent of the 'nodes' directory in the c++ version
export class Node {
  constructor(name, pretty, desc) {
    this._ctx = null;
    this._parent = null;
    this._name = name;
    this._pretty = pretty;
    this._desc = desc;
  }

  setParent(parent) { this._parent = parent; }
  setContext(ctx) { this._ctx = ctx; }

  getName() { return this._name; }
  getPretty() { return this._pretty; }
  getDesc() { return this._desc; }
  getParent() { return this._parent; }
  getContext() { return this._ctx; }

  path() {
    if (this._parent) {
      var p = this._parent.path();
      p.push(this._name);
      return p;
    } else {
      return [];
    }
  }

  fromPath(path, idx=0) {
    if (path.length <= idx) return this;
  }

  writeData(data) {
    return this._ctx ? this._ctx.writeData(this, data) : null;
  }

  data() {
    return this._ctx ? this._ctx.queryData(this) : null;
  }

  close() { return null; }
  pack() { return null; }

  *nodes() {
    yield this;
  }

  static unpack(proto) {
    switch(proto.node) {
    case 'group': return Group.unpack(proto);
    case 'var': return Variable.unpack(proto);
    case 'action': return Action.unpack(proto);
    case 'stream': return Stream.unpack(proto);
    }
  }
}

export class Group extends Node {
  constructor(name, pretty='', desc='',
              schema='', version=1, children=[]) {
    super(name, pretty, desc);
    this._schema = schema;
    this._version = version;
    this._children = children;
    this._childrenMap = new Map(); // name to child map

    for (let c of this._children) {
      c.setParent(this);
      this._childrenMap.set(c.getName(), c);
    }
  }

  setContext(ctx) {
    this._ctx = ctx;
    for (let c of this._children) c.setContext(ctx);
  }

  getSchema() { return this._schema; }
  getVersion() { return this._version; }
  getChildren() { return this._children; }

  fromPath(path, idx=0) {
    if (path.length <= idx) return this;
    else {
      var c = this._childrenMap.get(path[idx]);
      if (!c) return null;
      return c.fromPath(path, idx+1);
    }
  }

  child(child_name) {
    var c = this._childrenMap.get(child_name);
    return c;
  }

  *nodes() {
    yield this;
    for (let c of this._children) {
      for (let n of c.nodes()) {
        yield n;
      }
    }
  }

  clone() {
    return new Group(this._name, this._pretty, this._desc,
                     this._schema, this._version,
                     this._children.map(c => c.clone()));
  }

  pack() {
    return { group: { name: this._name, desc: this._desc,
             pretty: this._pretty, schema: this._schema, version: this._version,
             children: this._children.map((c) => c.pack()) } };
  }

  static unpack(proto) {
    var g = proto.group;
    return new Group(g.name, g.pretty, g.desc, g.schema, g.version,
                     g.children.map(n => Node.unpack(n)));
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
  constructor(name, pretty='', desc='', type=Type.NONE) {
    super(name, pretty, desc);
    this._type = type;
  }

  getType() { return this._type; }

  subscribe(minInterval, maxInterval) {
    return this._ctx ? this._ctx.subscribe(this, minInterval, maxInterval) : null;
  }

  feed() {
    return this._ctx ? this._ctx.feed(this) : null;
  }

  clone() {
    return new Variable(this._name, this._pretty, this._desc, this._type);
  }

  pack() {
    return { var: { name: this._name, pretty: this._pretty, desc: this._desc,
                    dataType: this._type.pack() } };
  }

  static unpack(proto) {
    var v = proto.var;
    return new Variable(v.name, v.pretty, v.desc, Type.unpack(v.dataType));
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
  }

  async call(arg) {
    return this._ctx ? this._ctx.call(this, arg) : null;
  }

  getArgType() { return this._argType; }
  getRetType() { return this._retType; }

  clone() {
    return new Action(this._name, this._pretty, this._desc,
                      this._argType, this._retType);
  }

  pack() {
    return { action: { name: this._name, pretty: this._pretty, desc: this._desc,
                       argType: this._argType.pack(), retType: this._retType.pack() } };
  }

  static unpack(proto) {
    var a = proto.action;
    return new Action(a.name, a.pretty, a.desc,
      Type.unpack(a.argType), Type.unpack(a.retType));
  }

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

  clone() {
    throw new Error("Stream type not implemented!");
  }

  pack() {
    throw new Error("Stream type not implemented!");
  }

  static unpack(proto) {
    throw new Error("Stream type not implemented!");
  }

  toString(indent = 0) {
    let line = ' '.repeat(indent) +
        this.getName() + ' stream (' + this.getPretty() + '): ' + this.getDesc();
    return line;
  }
}

export class Feed {
  constructor(set) {
    this.all = set;
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

  getNamespace() { return this._ns; }

  getName() { return this._name; }
  getUUID() { return this._uuid; }
  getType() { return this._type; }
  getInfo() { return this._info; }

  // to be overridden
  async fetch() { return null; }

  // returns a feed object with mounts
  async mounts(as_src=true, as_tgt=true) { return null; }

  // mount this context on another context
  async mount(src) { return false; }
  async unmount(src) { return false; }

  // context actions (params can either be path or node)
  async subscribe(variable, minInterval, maxInterval) { return null; }
  async call(action, arg) {}
  async writeData(node, data) { return false; }
  async queryData(node) {}

  async destroy() { return false; }

  pack() {
    return { ns: this._ns.getUUID(),
           uuid: this.getUUID(),
           name: this.getName(),
           type: this.getType(),
           info: Info.pack(this.getInfo()) }
  }

  toString() {
    return this._name;
  }
}

class Task {
  constructor(type) {
    this._type = type;
  }

  getType() { return this._type; }
}

// a namespace can handle all of the operations
// defined in api.proto. A relay then exposes a namespace.
export class Namespace {
  constructor(uuid) { this._uuid = uuid; this.destroyed = new Signal() }

  getUUID() { return this._uuid; }

  // returns all mounts, if srcs_uuid or tgts_uuid is
  // specified it will show the srcs/tgts of those contexts
  async mounts({srcs_uuid=null, tgts_uuid=null}) {  return null; }
  async contexts({by_uuid=null, by_name=null, by_type=null}) { return null; }
  async tasks({type=null}) { return null; }

  async fetch(ctxUuid, ctx=null) { return null; }

  async subscribe(ctxUuid, path, minInterval, maxInterval) { return null; }
  async call(ctxUuid, path, arg) { return null; }

  async data(ctxUuid, path) { return null; }
  async writeData(ctxUuid, path, data) { return null; }

  async mount(src, tgt) { return false; }
  async unmount(src, tgt) { return false; }

  async createContext(name, type, info, sources) { return null; }
  async destroyContext(ctxUuid) { return false; }

  async spawnTask(name, type, info, sources) { return null; }
  async killTask(taskUUID) { return false; }
}

export var Info = {
  pack(json) {
    var type = typeof json;
    if (Array.isArray(json)) {
      return {isArray: true, array: json.map((x) => Info.pack(x)), object: []};
    } else if (type == "number") {
      return {number:json, array: [], object: []};
    } else if (type == "string") {
      return {str:json, array: [], object: []};
    } else if (type == "object") {
      return { array: [], isObject: true,
               object: Object.entries(json).map(
                 ([k,v]) => { return { key: k, value: Info.pack(v) } } ) };
    }
    return null;
  },
  unpack(protobuf) {
    if (protobuf == null) return null;
    if (protobuf.isArray) {
      return protobuf.array.map(Info.unpack);
    } else if (protobuf.isObject) {
      var obj = {};
      for (let {key, value} of protobuf.object) {
        obj[key] = Info.unpack(value);
      }
      return obj;
    } else {
      return protobuf.number != undefined ? protobuf.number :
             protobuf.str != undefined ? protobuf.str :
             protobuf.object != undefined ? Info.unpack(protobuf.object) : null;
    }
  }
}

export var Value = {
  pack(val) {
    var t = typeof val;
    if (t == 'boolean') return { b: val };
    if (t == 'number') {
      if (Number.isInteger(val)) {
        if (val > 0) return { uint: val };
        else return { sint: val };
      } else {
        return { f: val };
      }
    }
  },
  unpack(proto) {
    if (proto.b) return proto.b;
    if (proto.en) return { en: val.enum };
    if (proto.uint) return proto.uint;
    if (proto.sint) return proto.sint;
    if (proto.f) return proto.f;

    // for things that don't fit into
    // javascript numbers, just parse them for now
    if (proto.ulong) return parseInt(proto.ulong);
    if (proto.slong) return parseInt(proto.slong);
    if (proto.d) return parseFloat(proto.d);
    return null;
  }
}
