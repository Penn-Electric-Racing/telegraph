import { Signal } from './signal.mjs'

export class Collection {
  constructor(parent=null) {
    this.current = new Map();
    this.added = new Signal();
    this.removed = new Signal();

    // parents have weak
    // references to their children, but
    // make children have strong references
    // to their parents so that the parents
    // are kept around
    this._parent = parent;
  }

  get empty() {
    return this.current.size == 0;
  }

  get size() {
    return this.current.size;
  }

  bind(array) {
    this.added.addWeak(array, (x) => array.push(x));
    this.removed.addWeak(array, (x) => array.splice(array.indexOf(x), 1));
  }

  unbind(array) {
    this.added.removeWeakAll(array);
    this.removed.removeWeakAll(array);
  }

  // returns a filtered collection,
  // which is connected to this collection
  // by weak reference
  filter(f) {
    var filtered = new Collection(this);
    for (let o of this.current.values()) {
      if (f(o)) filtered._add(o);
    }
    // add weakly so if the filtered version is dropped,
    // there are no reprocussions for us keeping them around
    this.added.addWeak(filtered, (w, o) => { if (f(o)) w._add(o) });
    this.removed.addWeak(filtered, (w, o) => { w._remove(o); });
    return filtered;
  }

  unwrap(strict=true) {
    if (strict && this.size != 1) return null;
    else if (this.size == 0) return null;
    return this.current.values().next().value;
  }

  extract(f) {
    for (let o of this.current.values()) {
      if (f(o)) {
        return o;
      }
    }
    return null;
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

  // to be used internally only...

  _add(o) {
    this.current.set(o.uuid, o);
    this.added.dispatch(o);
  }

  _remove(o) {
    if (this.current.has(o.uuid)) {
      this.current.delete(o.uuid);
      this.removed.dispatch(o);
    }
  }

  _removeUUID(u) {
    if (this.current.has(u)) {
      var o = this.current.get(u);
      this.current.delete(u);
      this.removed.dispatch(o);
    }
  }

  _clear() {
    for (let o of this.current.values()) {
      this.removed.dispatch(o);
    }
    this.current.clear();
  }
}
