import { Signal } from './signal.mjs'

export class Collection {
  constructor() {
    this.current = new Map();
    this.added = new Signal();
    this.removed = new Signal();
  }

  get empty() {
    return this.current.size == 0;
  }

  get size() {
    return this.current.size;
  }

  filter(f) {
    var filtered = new Collection();
    for (let o of this.current.values()) {
      if (f(o)) filtered.add(o);
    }
    // add weakly so if the filtered version is dropped,
    // there are no reprocussions for us keeping them around
    this.added.addWeak(filtered, (w, o) => { if (f(o)) w.add(o) });
    this.removed.addWeak(filtered, (w, o) => { w.removeUUID(o); });
    return filtered;
  }

  unwrap() {
    if (this.size != 1) throw new Error("Cannot unwrap collection of size > 1");
    return this.current.values().next().value;
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

  removeUUID(u) {
    if (this.current.has(u)) {
      var o = this.current.get(u);
      this.current.delete(u);
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

  get(uuid) {
    return this.current.get(uuid);
  }

  [Symbol.iterator]() {
    return this.current.values();
  }
}
