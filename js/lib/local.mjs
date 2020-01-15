import { Namespace, Query, Context } from './namespace.mjs'
import { Node } from './nodes.mjs'
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

    // mounts set
    this._mounts = new Set();
    this._mountAdded = new Signal();
    this._mountRemoved = new Signal();

    // uuid -> task
    this._tasks = new Map();
    this._taskAdded = new Signal();
    this._taskRemoved = new Signal();
  }

  // querying functions
  async contexts({byUuid=null, byName=null, byType=null}) {
    var filter = (ctx) => (!byUuid || ctx.getUUID() == byUuid) && 
                          (!byName || ctx.getName() == byName) &&
                          (!byType || ctx.getType() == byType);
    var filtered = new Set();
    for (let c of this._contexts.values()) {
      if (filter(c)) filtered.add(c);
    }

    var q = new Query(filtered);
    var al = (c) => { if (filter(c)) { filtered.add(c); q.added.dispatch(c); } };
    var rl = (c) => { if (filter(c)) { filtered.remove(c); q.removed.dispatch(c); } };
    this._contextAdded.add(al);
    this._contextRemoved.add(rl);
    q.closed.add(() => { this._contextAdded.remove(al); this._contextRemoved.remove(rl); });
    return q;
  }

  async mounts({srcsOf=null, tgtsOf=null}) {
    var filter = (m) => (srcsOf && m.tgt == srcsOf) ||
                        (tgtsOf && m.src == tgtsOf) ||
                        (!srcsOf && !tgtsOf);
    var filtered = new Set();
    for (let m of this._mounts) {
      if (filter(m)) filtered.add(m);
    }
    var q = new Query(filtered);
    var al = (m) => { if (filter(m)) { filtered.add(m); q.added.dispatch(m); } };
    var rl = (m) => { if (filter(m)) { filtered.remove(m); q.removed.dispatch(m); } };
    this._mountAdded.add(al);
    this._mountRemoved.add(rl);
    q.closed.add(() => { this._mountAdded.remove(al); this._mountRemoved.remove(rl); });
    return q;
  }

  async tasks({byUuid=null, byName=null, byType=null}) {
    var filter = (task) => (!byUuid || task.getUUID() == byUuid) && 
                           (!byName || task.getName() == byName) &&
                           (!byType || task.getType() == byType);
    var filtered = new Set();
    for (let t of this._tasks) {
        console.log(t);
      if (filter(t)) filtered.add(t);
    }
  }

  async fetch(uuid, ignoredCtx=null) {
    var ctx = this._contexts.get(uuid);
    if (!ctx) return null;
    return await ctx.fetch();
  }

  async subscribe(ctxUuid, path, minInterval, maxInterval) {
    var ctx = this._contexts.get(ctxUuid);
    if (!ctx) return null;
    return ctx.subscribe(path, minInterval, maxInterval);
  }
}

export class LocalContext extends Context {
  constructor(ns, name, type, info, tree) {
    if (tree.getContext())
      throw new Error("Cannot use the same tree for two contexts");

    super(ns, uuidv4(), name, type, info);
    this._tree = tree;
    this._tree.setContext(this);
    ns._contexts.set(this.getUUID(), this);
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

    var f = new Query(initial);
    var al = (m) => { if (filter(m)) { filtered.add(m); f.added.dispatch(m); } };
    var rl = (m) => { if (filter(m)) { filtered.remove(m); f.removed.dispatch(m); } };
    ns._mountAdded.add(al);
    ns._mountRemoved.add(rl);
    f.closed.add(() => { ns._mountAdded.remove(al); ns._mountRemoved.remove(rl); });
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
    if (!(variable instanceof Node)) {
      variable = this._tree.fromPath(variable);
    }
    if (!variable) return null;
    if (!this._publishers.has(variable)) return null;
    else return this._publishers.get(variable).subscribe(minInterval, maxInterval);
  }

  async call(action, arg) {
    if (!(action instanceof Node)) {
      action = this._tree.fromPath(action);
    }
    if (!action) return null;
    if (this._actions.has(action)) return await this._actions.get(action)(arg);
    else return undefined;
  }
}

export class LocalContainer extends LocalContext {
  constructor(ns, name, tree) {
    super(ns, name, 'container', {}, tree);
    this._srcs = [];
  }

  mount(src) {
    if (this._ns._addMount(src, this)) {
      this._srcs.push(src);
      return true;
    }
    return false;
  }

  unmount(src) {
    this._srcs.splice(this._srcs.indexOf(src), 1);
    return this._ns._removeMount(src, this);
  }

  async subscribe(variable, minInterval, maxInterval) {
    if (variable instanceof Node) variable = variable.path();
    for (let src of this._srcs) {
      var sub = await src.subscribe(variable, minInterval, maxInterval);
      if (sub) return sub;
    }
    return null;
  }
}

export class LocalArchive extends LocalContext {
  constructor(ns, name, tree) {
    super(ns, name, 'archive', {}, tree);
  }
}
