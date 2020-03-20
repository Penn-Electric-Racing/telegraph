export class Signal {
  constructor() {
    this.listeners = [];
    this.weakListeners = new Map();
  }

  add(f) {
    this.listeners.push(f);
  }

  remove(f) {
    this.listeners.splice(this.listeners.indexOf(f), 1);
  }

  // TODO: Use actual WeakRef!
  addWeak(o, f) {
    if (!this.weakListeners.get(o)) this.weakListeners.set(o, []);
    this.weakListeners.get(o).push(f);
  }

  removeWeak(o, f) {
    var c = this.weakListeners.get(o);
    c.splice(c.indexOf(f), 1);
  }

  dispatch(...args) {
    for (let l of this.listeners) {
      l(...args);
    }
    for (let [o, f] of this.weakListeners) {
      f(o, ...args);
    }
  }
}
