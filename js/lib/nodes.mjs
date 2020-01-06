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

