import { Signal } from './signal.mjs'

// the query API
// allows for clients to build query objects and receive deltas
// when objects match those queries/do not match those queries

// that way resources can be referenced (i.e contexts/nodes) even when they are not yet available
// due to i.e the client losing connection

// you call update() on a query to feed in a resource
// so for a NamespaceQuery update() will

// represents a node query


export class Query {
  constructor(parent=null) {
    this.parent = parent;
    this.current = null;
    this.updated = new Signal();
  }
  update(newValue) {
    var old = this.current;
    this.current = newValue;
    this.updated.dispatch(newValue, old);
  }
}

export class TaskQuery extends Query {

}

export class TasksQuery extends Query {

}

export class MountQuery extends Query {

}

export class MountsQuery extends Query {

}

export class NodeQuery extends Query {
  constructor(parent=null) {
    super(parent);
  }
}

export class ContextQuery extends Query {
  constructor(parent=null) {
    super(parent);
  }

  fetch() {
    var q = new NodeQuery(this);

    async function update(query, n, o) {
      if (n) {
        var tree = await n.fetch();
        query.update(tree);
      } else {
        query.update(null);
      }
    }

    update(q, this.current, null);
    this.updated.addWeak(q, update);
    return q;
  }
}

export class ContextsQuery extends Query {
  constructor(parent=null) {
    super(parent);
  }

  unwrap() {
    var cq = new ContextQuery(this);
    var self = this;

    function trigger(query) {
      var contexts = self.current;
      query.update(contexts ? contexts.unwrap() : null);
    }

    // update will add callbacks so that trigger()
    // calls every time contexts gets an add or remove
    function update(query, newCollection, oldCollection) {
      if (oldCollection) {
        oldCollection.added.removeWeak(query, trigger);
        oldCollection.removed.removeWeak(query, trigger);
      }
      if (newCollection) {
        newCollection.added.addWeak(query, trigger);
        newCollection.removed.addWeak(query, trigger);
      }
      trigger(query, newCollection);
    }

    update(cq, this.current, null);
    this.updated.addWeak(cq, update);
    return cq;
  }

  filter(f) {
    var cq = new ContextsQuery(this);

    function update(query, n, o) { query.update(n ? n.filter(f) : null); }
    update(cq, this.current, null);
    this.updated.addWeak(cq, update);
    return cq;
  }
}


//
export class NamespaceQuery extends Query {
  constructor(parent=null) {
    super(parent);
  }

  get tasks() {
    var q = new TasksQuery(this);
    function update(query, n, o) { query.update(n ? n.tasks : null); }

    update(q, this.current, null);
    this.updated.addWeak(q, update);
    return q;
  }

  get contexts() {
    var q = new ContextsQuery(this);
    function update(query, n, o) { query.update(n ? n.contexts : null); }

    update(q, this.current, null);
    this.updated.addWeak(q, update);
    return q;
  }

  get mounts() {
    var q = new MountsQuery(this);
    function update(query, n, o) { query.update(n ? n.mounts : null); }

    update(q, this.current, null);
    this.updated.addWeak(q, update);
    return q;
  }
}

