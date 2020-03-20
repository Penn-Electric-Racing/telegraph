import { Signal } from './signal.mjs'

export class Container {
  constructor() {
    this.current = new Map();
    this.added = new Signal();
    this.removed = new Signal();
  }

  add(o) {
    this.current.set(o.uuid, o);
    this.added.dispatch(o);
  }

  remove(o) {
    if (this.current.has(o.uuid)) {
      this.current.delete(o.uuid);
      this.removed.dispatch(o);
    }
  }

  clear() {
    for (let o of this.current.values()) {
      this.removed.dispatch(o);
    }
    this.current.clear();
  }

  has(o) {
    return this.current.has(o.uuid);
  }

  hasUUID(uuid) {
    return this.current.has(uuid);
  }

  [Symbol.iterator]() {
    return this.current.values();
  }
}
