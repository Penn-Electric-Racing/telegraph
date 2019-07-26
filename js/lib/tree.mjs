

class Node {
  constructor(name) {
    this.name = name;
  }
}

class Group extends Node {
  constructor(name, schema, version) {
    super(name);
    this.schema = schema;
    this.version = version;
  }
}

class Variable {
}

class Action {
}

class Stream {
}

class Tree {
}
