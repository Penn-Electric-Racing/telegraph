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
    if (idx > path.length) return null;
    else if (idx == path.length) return this;
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

  get valid() { return this._class != Type.INVALID._class; }

  pack() {
    var type = null;
    switch (this._class) {
      case Type.NONE._class:   type = 1; break;
      case Type.ENUM._class:   type = 2; break;
      case Type.BOOL._class:   type = 3; break;
      case Type.UINT8._class:  type = 4; break;
      case Type.UINT16._class: type = 5; break;
      case Type.UINT32._class: type = 6; break;
      case Type.UINT64._class: type = 7; break;
      case Type.INT8._class:   type = 8; break;
      case Type.INT16._class:  type = 9; break;
      case Type.INT32._class:  type = 10; break;
      case Type.INT64._class:  type = 11; break;
      case Type.FLOAT ._class: type = 12; break;
      case Type.DOUBLE._class: type = 13; break;
      default: type = 0; break; // invalid
    }
    return {name: this._name, type: type, labels: this._labels};
  }

  static unpack(proto) {
    switch(proto.type) {
      case 0: return Type.INVALID;
      case 1: return Type.NONE;
      case 2: return new Type(proto.name, proto.labels, Type.ENUM._class);
      case 3: return Type.BOOL;
      case 4: return Type.UINT8;
      case 5: return Type.UINT16;
      case 6: return Type.UINT32;
      case 7: return Type.UINT64;
      case 8: return Type.INT8;
      case 9: return Type.INT16;
      case 10: return Type.INT32;
      case 11: return Type.INT64;
      case 12: return Type.FLOAT;
      case 13: return Type.DOUBLE;
      default: return Type.INVALID;
    }
  }

  toString() {
    switch (this._class) {
      case Type.BOOL._class:   return 'bool';
      case Type.NONE._class:   return 'none';
      case Type.ENUM._class:   return 'enum ' + this._name + ' [' + this._labels.join(', ') + ']';
      case Type.UINT8._class:  return 'uint8';
      case Type.UINT16._class: return 'uint16';
      case Type.UINT32._class: return 'uint32';
      case Type.UINT64._class: return 'uint64';
      case Type.INT8._class:   return 'int8';
      case Type.INT16._class:  return 'int16';
      case Type.INT32._class:  return 'int32';
      case Type.INT64._class:  return 'int64';
      case Type.FLOAT._class:  return 'float';
      case Type.DOUBLE._class: return 'double';
      default: return 'invalid';
    }
  }
}

export var Value = {
  pack(val, type) {
    if (!type) return {};
    switch(type._class) {
      case Type.NONE._class:   return { none: {} };
      case Type.BOOL._class:   return { b: val };
      case Type.ENUM._class:   return { en: val };
      case Type.UINT8._class:  return { u8: val };
      case Type.UINT16._class: return { u16: val };
      case Type.UINT32._class: return { u32: val };
      case Type.UINT64._class: return { u64: val };
      case Type.INT8._class:   return { i8: val };
      case Type.INT16._class:  return { i16: VAL };
      case Type.INT32._class:  return { i32: val };
      case Type.INT64._class:  return { i64: val };
      case Type.FLOAT._class:  return { f: val };
      case Type.DOUBLE._class: return { d: val };
    }
  },
  unpack(proto) {
    if (proto.f) return proto.f;
    if (proto.d) return proto.d;
    if (proto.b) return proto.b;
    if (proto.en) return proto.en;
    if (proto.u8) return proto.u8;
    if (proto.u16) return proto.u16;
    if (proto.u32) return proto.u32;
    if (proto.u64) return proto.u64;
    if (proto.i8) return proto.i8;
    if (proto.i16) return proto.i16;
    if (proto.i32) return proto.i32;
    if (proto.i64) return proto.i64;
    return {}; // return none type
  }
}
