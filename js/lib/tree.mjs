import Signal from 'signals'

// the equivalent of the 'nodes' directory in the c++ version
export class Node {
  constructor(name, pretty, desc) {
    this.name = name;
    this.pretty = pretty;
    this.desc = desc;
    this.parent = null;
    this.on_parent_changed = new Signal();
  }

  inspect() {
    return this.toString();
  }
}

export class Group extends Node {
  constructor(name, pretty, desc, schema, version) {
    super(name, pretty, desc);
    this.schema = schema;
    this.version = version;
    this.children = [];
    this._children_map = new Map(); // name to child map

    this.on_add_child = new Signal();
    this.on_remove_child = new Signal();
  }

  get(child_name) {
    var c = this._children_map.get(child_name);
    return c;
  }

  find(path) {
    var parts = path.split('/');
    var current = this;
    for (let p of parts) {
      if (!current) return null;
      if (current instanceof Group && p.length > 0) {
        current = current.get(p);
      }
    }
    return current;
  }

  add_child(child, notify_parent_changed=true) {
    if (child == null || child.parent == this) return;
    if (this._children_map.has(child.name)) throw new Error("Child with the same name already exists");
    if (child.parent != null) {
      child.parent.remove_child(child, false);
    }
    this.children.push(child);
    this._children_map.set(child.name, child);
    child.parent = this;
    if (notify_parent_changed) child.on_parent_changed.dispatch(this);
    this.on_add_child.dispatch(child);
  }

  remove_child(child, notify_parent_changed=true) {
    if (child == null) return;
    let i = this.children.indexOf(child);
    if (i >= 0) {
      this.children.splice(i, 1);
      this._children_map.remove(child.name);
      child.parent = null;
      if (notify_parent_changed) child.on_parent_changed.dispatch(null);
      this.on_remove_child.dispatch(child);
    }
  }

  toString(indent = 0) {
    let line = ' '.repeat(indent) + 
        this.name + ' ' + this.schema + '/' + this.version + 
          ' (' + this.pretty + '): ' + this.desc;

    // add the children
    if (this.children.length > 0) 
      line += '\n' + this.children.map(c => c.toString(indent + 4)).join('\n');

    return line;
  }
}

export class Type {
  constructor(ident, name = null, labels = []) {
    this._ident = ident;
    this._name = name;
    this._labels = labels;
  }

  pack() {
    if (this._ident == Type.ENUM._ident) {
      return {type: "enum", labels : this._labels};
    } else {
      switch (this._ident) {
        case Type.BOOL._ident:   return "bool";
        case Type.UINT8._ident:  return "uint8";
        case Type.UINT16._ident: return "uint16";
        case Type.UINT32._ident: return "uint32";
        case Type.UINT64._ident: return "uint64";
        case Type.INT8._ident:   return "int8";
        case Type.INT16._ident:  return "int16";
        case Type.INT32._ident:  return "int32";
        case Type.INT64._ident:  return "int64";
        case Type.FLOAT ._ident:  return "float";
        case Type.DOUBLE._ident:  return "double";
        default: return "invalid";
      }
    }
  }
}

Type.unpack = function(json) {
  if (json instanceof String) {
    switch(json) {
      case "invalid" : return Type.INVALID;
      case "bool" : return Type.BOOL;
      case "uint8" : return Type.UINT8;
      case "uint16" : return Type.UINT16;
      case "uint32" : return Type.UINT32;
      case "uint64" : return Type.UINT64;
      case "int8" : return Type.INT8;
      case "int16" : return Type.INT16;
      case "int32" : return Type.INT32;
      case "int64" : return Type.INT64;
      case "float" : return Type.FLOAT;
      case "double" : return Type.DOUBLE;
    }
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

export class Variable extends Node {
  constructor(name, pretty, desc, type) {
    super(name, pretty, desc);
    this._type = type;
    this._source = null;
  }

  getType() { return this._type; }

  setSource(source) {
    this._source = source;
  }

  subscribe(min_interval, max_interval) {
    if (!this._source) return null;
    return this._source.subscribe(this, min_interval, max_interval);
  }
}

export class Action {
}

export class Stream {
}

export class Tree {
  constructor(root) {
    this.root = root;
  }

  find(path) {
    if (this.root) return this.root.find(path);
    else return null;
  }

  inspect() {
    return this.toString();
  }

  toString() {
    return this.root.toString();
  }
}
