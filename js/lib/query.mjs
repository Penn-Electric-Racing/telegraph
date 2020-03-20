// the query API
// allows for clients to build query objects and receive deltas
// when objects match those queries/do not match those queries

// that way resources can be referenced (i.e contexts/nodes) even when they are not yet available
// due to i.e the client losing connection

// you call update() on a query to feed in a resource
// so for a NamespaceQuery update() will

// represents a node query
export class NodeQuery {
  constructor(node) {
  }
  update(node) {
  }
}

export class ContextQuery {
}

export class ContextsQuery {
}

export class TaskQuery {
}

export class TasksQuery {
}

export class MountQuery {
}

export class MountsQuery {
}

//
export class NamespaceQuery {
  constructor(ns) {
    this.ns = ns;
    this.onChange = new Signal();
  }

  tasks() {
    var q = new TasksQuery(this.ns ? this.ns.tasks : null);
    // weak add so this query can be cleaned up automagically!
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.tasks : null));
  }

  contexts() {
    var q = new ContextsQuery(this.ns ? this.ns.contexts : null);
    // weak add so this query can be cleaned up automagically!
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.contexts : null));
  }

  mounts() {
    var q = new MountsQuery(this.ns ? this.ns.mounts : null);
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.mounts : null));
  }

  update(ns) {
    this.ns = ns;
    this.onChange.dispatch(client);
  }
}

