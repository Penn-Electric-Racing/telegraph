import Signal from 'signals'

// the equivalent of the 'nodes' directory in the c++ version
export class Node {
  constructor(name, pretty, desc) {
    this.name = name;
    this.pretty = pretty;
    this.desc = desc;
    this.parent = null;

    this.onDescendantAdded = new Signal();
    this.onDescendantRemoved = new Signal();
    this.onDispose = new Signal();
  }

  dispose() {
    this.onDispose.dispatch();
  }

  getName() { return this.name; }
  getPretty() { return this.pretty; }
  getDesc() { return this.desc; }
  getParent() { return this.parent; }

  inspect() {
    return this.toString();
  }
}

export class Group extends Node {
  constructor(name, pretty, desc, schema, version) {
    super(name, pretty, desc);
    this._schema = schema;
    this._version = version;
    this._children = [];
    this._childrenMap = new Map(); // name to child map

    this.onAddChild = new Signal();
    this.onRemoveChild = new Signal();
  }

  getSchema() { return this._schema; }
  getVersion() { return this._version; }

  getChildren() { return this._children; }

  dispose() {
    for (let c of this._children) {
      c.dispose();
    }
    this.onDispose.dispatch();
  }

  * descendants() {
      for (let c of this._children) {
          yield c;
          if (c instanceof Group) {
              for (let d of c.descendants()) {
                  yield d;
              }
          }
      }
  }


  child(child_name) {
    var c = this._childrenMap.get(child_name);
    return c;
  }

  find(path) {
    var parts = path.split('/');
    var current = this;
    for (let p of parts) {
      if (!current) return null;
      if (current instanceof Group && p.length > 0) {
        current = current.child(p);
      }
    }
    return current;
  }

  addChild(child) {
    if (child == null || child.parent == this) return;
    if (this._childrenMap.has(child.name)) throw new Error("Child with the same name already exists");

    if (child.parent != null) {
      child.parent.removeChild(child, false);
    }

    this._children.push(child);
    this._childrenMap.set(child.name, child);
    child.parent = this;

    // notify this group's listeners
    this.onAddChild.dispatch(child);

    // notify the tree that nodes have been added
    var dispatch = function (node) {
        this.onDescendantAdded.dispatch(node);
        if (node instanceof Group) {
            for (let c of node.getChildren()) {
                dispatch(c);
            }
        }
    }.bind(this);
    dispatch(child);

    child.onDescendantAdded.add(this.onDescendantAdded.dispatch);
    child.onDescendantRemoved.add(this.onDescendantRemoved.dispatch);
  }

  removeChild(child) {
    if (child == null) return;
    let i = this._children.indexOf(child);
    if (i >= 0) {
      this._children.splice(i, 1);
      this._childrenMap.remove(child.name);
      child.parent = null;
      this.onRemoveChild.dispatch(child);
      // notify the tree that nodes have been added
      var dispatch = function (node) {
          if (node instanceof Group) {
              for (let c of node.getChildren()) {
                  dispatch(c);
              }
          }
          this.onDescendantRemoved.dispatch(node);
      }.bind(this);
      dispatch(child);
    }
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

  unpackValue(val) {
    var x = undefined;
    switch(this._ident) {
      case Type.NONE._ident:   x = null; break;
      case Type.BOOL._ident:   x = val.b; break;
      case Type.ENUM._ident:   x = val.en; break;
      case Type.UINT8._ident:  x = val.ui8; break;
      case Type.UINT16._ident: x = val.ui16; break;
      case Type.UINT32._ident: x = val.ui32; break;
      case Type.UINT64._ident: x = val.ui64; break;
      case Type.INT8._ident:   x = val.i8; break;
      case Type.INT16._ident:  x = val.i16; break;
      case Type.INT32._ident:  x = val.i32; break;
      case Type.INT64._ident:  x = val.i64; break;
      case Type.FLOAT ._ident: x = val.f; break;
      case Type.DOUBLE._ident: x = val.d; break;
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
      case Type.FLOAT._ident: return 'float';
      case Type.DOUBLE._ident: return 'double';
      default: return 'invalid';
    }
  }
}

Type.unpack = function(json) {
  switch(json.type) {
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
    case "ENUM": return new Type(Type.ENUM._ident, json.name, json.labels);
    default: return Type.INVALID;
  }
};

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

/*
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
*/

export class Tree {
  constructor(root) {
    if (!root) throw new Error("Root of a tree cannot be null");

    this.root = root;
    this.onNodeAdded = new Signal();
    this.onNodeRemoved = new Signal();
    this.root.onDescendantAdded.add(this.onNodeAdded.dispatch);
    this.root.onDescendantRemoved.add(this.onNodeRemoved.dispatch);
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
