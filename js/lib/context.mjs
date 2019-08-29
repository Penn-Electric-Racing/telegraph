import Signal from 'signals'

export class Context {
  constructor(name, tree) {
    this._name = name;
    this._setTree(tree);
    this.onDispose = new Signal();
  }

  _setTree(tree) {
    if (!tree) return;
    if (tree instanceof Function) {
      this._tree_getter = tree;
      this._tree = null;
    } else {
      this._tree = tree;
      this._tree_getter = null;
    }
  }

  onDispose(func) { this._disposers.push(func); }

  getName() { return this._name; }

  async get() {
    if (this._tree) return this._tree;
    else {
      this._tree = (await this._tree_getter());
      return this._tree;
    }
  }

  // will close any connections related to context
  dispose() {
    if (this._tree) this._tree.dispose();
    this._tree_getter = null;
    this._tree = null;
    this.onDispose.dispatch();
  }

  toString() {
    return this._name;
  }
}
