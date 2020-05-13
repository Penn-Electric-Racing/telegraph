import { Signal } from './signal.mjs'
import { Collection } from './collection.mjs';

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
    if (this.current == newValue) return;
    var old = this.current;
    this.current = newValue;
    this.updated.dispatch(newValue, old);
  }
}

// common class for queries operating
// on collections
// (i.e component queries, context queries, etc.)
class CollectionsQuery extends Query {
  constructor(queryType, selfType, parent=null) {
    super(parent);
    this.queryType = queryType;
    this.selfType = selfType;
  }
  unwrap() {
    var tq = new this.queryType(this);
    var self = this;

    function trigger(query) {
      var components = self.current;
      query.update(components ? components.unwrap() : null);
    }

    // update will add callbacks so that trigger()
    // calls every time contexts gets an add or remove
    function onUpdate(query, newCollection, oldCollection) {
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
    onUpdate(tq, this.current, null);
    this.updated.addWeak(tq, onUpdate);
    return tq;
  }

  collect() {
    var c = new Collection(this);
    var self = this;

    var onAdd = function(col, obj) {
      col._add(obj);
    }
    var onRemove = function(col, obj) {
      col._remove(obj);
    }
    var onUpdate = function (col, newCollection, oldCollection) {
      if (oldCollection) {
        oldCollection.added.removeWeak(col, onAdd);
        oldCollection.removed.removeWeak(col, onRemove);
        for (let c of oldCollection) {
          onRemove(col, c);
        }
      }
      if (newCollection) {
        newCollection.added.addWeak(col, onAdd);
        newCollection.removed.addWeak(col, onRemove);
        for (let c of newCollection) {
          onAdd(col, c);
        }
      }
    }
    onUpdate(c, this.current, null);
    this.updated.addWeak(c, onUpdate);
    return c;
  }

  filter(f) {
    var sq = new this.selfType(this);
    function onUpdate(query, n, o) { query.update(n ? n.filter(f) : null); }
    onUpdate(sq, this.current, null);
    this.updated.addWeak(sq, onUpdate);
    return sq;
  }

  extract(f) {
    return this.filter(f).unwrap();
  }
}

export class ComponentQuery extends Query {
  constructor(parent = null) {
    super(parent);
  }
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

export class MountQuery extends Query {

}

export class ComponentsQuery extends CollectionsQuery {
  constructor(parent = null) {
    super(ComponentQuery, ComponentsQuery, parent);
  }
}
export class MountsQuery extends CollectionsQuery {
  constructor(parent = null) {
    super(MountQuery, MountsQuery, parent);
  }
}
export class ContextsQuery extends CollectionsQuery {
  constructor(parent=null) {
    super(ContextQuery, ContextsQuery, parent);
  }
}

//
export class NamespaceQuery extends Query {
  constructor(parent=null) {
    super(parent);
  }

  get components() {
    var q = new ComponentsQuery(this);
    function update(query, n, o) { query.update(n ? n.components : null); }

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

