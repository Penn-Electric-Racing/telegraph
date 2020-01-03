export { Namespace, Context, Type, Variable,
         Node, Group, Action, Stream } from './lib/namespace.mjs'
export { LocalNamespace, LocalContext,
         LocalDevice, LocalContainer, LocalArchive } from './lib/local.mjs'
export { Relay } from './lib/remote.mjs'

// utility class that lets
// you fetch a single context easily
// with namespace changes
export class ContextRetriever {
  constructor(opts, cb) {
    this._ns = null;
    this._feed = null;
    this._ctx = null;
    this._opts = opts;
    this._cb = cb;
  }
  async namespaceChanged(ns) {
    if (this._feed) {
      await this._feed.close();
      this._feed = null;
      this._ns = null;
      this._ctx = null;
    }
    if (ns) {
      this._ns = ns;
      this._feed = await ns.contexts(this._opts);
      var ctxs = [...this._feed.all];
      if (ctxs.length > 0) {
        this._ctx = ctxs[0];
        this._cb(this._ctx);
      }
      this._feed.added.add(ctx => {
        if (!this._ctx) {
          this._ctx = ctx;
          this._cb(this._ctx);
        }
      });
      this._feed.removed.add(ctx => {
        if (this._ctx == ctx) {
          this._ctx = null;
          var ctxs = [...this._feed.all];
          if (ctxs.length > 0) this._ctx = ctxs[0];
          this._cb(this._ctx);
        }
      });
    }
  }
  async stop() {
    if (this._feed) {
      await this._feed.close();
      this._feed = null;
      this._ns = null;
    }
  }
}
