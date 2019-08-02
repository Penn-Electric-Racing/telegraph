import Signal from 'signals'

// the equivalent of the 'nodes' directory in the c++ version
export class Node {
  constructor(name, pretty, desc) {
    this.name = name;
    this.pretty = pretty;
    this.desc = desc;
    this.parent = null;
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

    this.on_add_child = new Signal();
    this.on_remove_child = new Signal();
  }

  add_child(child) {
    if (child.parent == this) return;
    if (child.parent != null) child.parent.remove_child(child);
    this.children.push(child);
    child.parent = this;
    this.on_add_child.dispatch(child);
  }

  remove_child(child) {
    this.children.remove(child);
    child.parent = null;
    this.on_remove_child.dispatch(child);
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

export class Variable {
}

export class Action {
}

export class Stream {
}

export class Tree {
  constructor(root) {
    this.root = root;
  }

  inspect() {
    return this.toString();
  }
  toString() {
    return this.root.toString();
  }
}
