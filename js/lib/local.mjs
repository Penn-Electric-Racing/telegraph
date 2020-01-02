import { Namespace, Feed, Context } from './namespace.mjs'
import uuidv4 from 'uuid'
import Signal from 'signals'

export class LocalNamespace extends Namespace {
  constructor(uuid) {
    if (!uuid) uuid = uuidv4()
    super(uuid)

    // uuid -> context
    this._contexts = new Map();
    this._contextAdded = new Signal();
    this._contextRemoved = new Signal();

    // map context -> set of contexts for whcih this is the target
    this._srcMounts = new Map();
    this._tgtMounts = new Map();
    this._mountAdded = new Signal();
    this._mountRemoved = new Signal();

    // task uuid to task
    this._tasks = new Map();
    this._taskAdded = new Signal();
    this._taskRemoved = new Signal();
  }

  _addLocalContext(ctx) {
    this._contexts.set(ctx.getUUID(), ctx);
    this._contextAdded.dispatch(ctx);
  }

  _removeLocalContext(ctx) {
    this._contexts.remove(ctx.getUUID());
    this._contextRemoved.dispatch(ctx);
  }

  _addMount(srcCtx, tgtCtx) {
    var mount = {src:srcCtx, tgt:tgtCtx};

    if (!this._srcMounts.has(tgtCtx)) this._srcMounts.set(tgtCtx, []);
    this._srcMounts.get(tgtCtx).push(mount);
    if (!this._tgtMounts.has(srcCtx)) this._tgtMounts.set(srcCtx, []);
    this._tgtMounts.get(srcCtx).push(mount);

    this._mountAdded.dispatch(mount);

    return true;
  }

  _removeMount(srcCtx, tgtCtx) {
    var tgts = this._tgtMounts.get(srcCtx);
    var srcs = this._srcMounts.get(tgtCtx);

    var mount = null;
    for (let c of tgts) {
      if (c.tgt == tgtCtx) { mount = c; break; }
    }
    if (!mount) return false;

    tgts.splice(tgts.indexOf(mount), 1);
    srcs.splice(srcs.indexOf(mount), 1);

    if (tgts.length == 0) this._tgtMounts.remove(srcCtx);
    if (srcs.length == 0) this._srcMounts.remove(tgtCtx);
    this._mountRemoved.dispatch(mount);
    return true;
  }

  // querying functions
  async contexts({by_uuid=null, by_name=null, by_type=null}) {
    var filter = (ctx) => (by_uuid ? ctx.getUUID() == by_uuid : true) && 
                          (by_name ? ctx.getName() == by_name : true) &&
                          (by_type ? ctx.getType() == by_type : true);
    var filtered = new Set();
    for (let c of this._contexts.values()) {
      if (filter(c)) filtered.add(c);
    }

    var f = new Feed(filtered);
    var al = (c) => { if (filter(c)) { filtered.add(c); f.added.dispatch(c); } };
    var rl = (c) => { if (filter(c)) { filtered.remove(c); f.removed.dispatch(c); } };
    this._contextAdded.add(al);
    this._contextRemoved.add(rl);
    f.closed.add(() => { this._contextAdded.remove(al); this._contextRemoved.remove(rl); });
    return f;
  }

  async mounts({srcs=null, tgts=null}) {
  }

  async fetch(uuid) {
    var ctx = this._contexts.get(uuid);
    if (!ctx) return null;
    return await ctx.fetch();
  }
}

export class LocalContext extends Context {
  constructor(ns, name, type, info, tree) {
    if (tree.getContext())
      throw new Error("Cannot use the same tree for two contexts");

    super(ns, uuidv4(), name, type, info);
    this._tree = tree;
    this._tree.setContext(this);
    ns._addLocalContext(this);
  }

  async fetch() { return this._tree; }

  async mounts(srcs=true, tgts=true) {
    var filter = (m) => (srcs ? m.tgt == this : false) || 
                        (tgts ? m.src == this : false);

    var initial = new Set();
    const ns = this._ns;
    var srcMounts = ns._srcMounts.get(this);
    var tgtMounts = ns._tgtMounts.get(this);
    if (srcMounts) srcMounts.forEach((m) => { if (filter(m)) initial.add(m) });
    if (tgtMounts) tgtMounts.forEach((m) => { if (filter(m)) initial.add(m) });

    var f = new Feed(initial);
    var al = (m) => { if (filter(m)) { filtered.add(m); f.added.dispatch(m); } };
    var rl = (m) => { if (filter(m)) { filtered.remove(m); f.removed.dispatch(m); } };
    ns._mountAdded.add(al);
    ns._mountRemoved.add(rl);
    f.closed.add(() => { ns.mountAdded.remove(al); ns.mountRemoved.remove(rl); });
  }

  async destroy() {
    this._ns._removeLocalContext(this);
  }
}

export class LocalDevice extends LocalContext {
  constructor(ns, name, port, bauds, tree) {
    super(ns, name, 'device', { port: port, available_bauds: bauds }, tree);
    this._publishers = new Map();
    this._actions = new Map();
  }

  addPublisher(node, publisher) {
    this._publishers.set(node, publisher);
  }

  addAction(node, action) {
    this._actions.set(node, action);
  }

  async subscribe(variable, minInterval, maxInterval) {
    if (this._publishers.has(variable)) return null;
    else return this._publishers.get(variable).subscribe(minInterval, maxInterval);
  }

  async call(action, arg) {
    if (this._actions.has(action)) return await this._actions.get(action)(arg);
    else return undefined;
  }
}

export class LocalContainer extends LocalContext {
  constructor(ns, name, tree) {
    super(ns, name, 'container', {}, tree);
  }

  mount(src) {
    return this._ns._addMount(src, this);
  }

  unmount(src) {
    return this._ns._removeMount(src, this);
  }
}

export class LocalArchive extends LocalContext {
  constructor(ns, name, tree) {
    super(ns, name, 'archive', {}, tree);
  }
}
