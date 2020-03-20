import { Signal } from './signal.mjs'
import { Container } from './container.mjs'
import { Connection } from './connection.mjs'

class Context {
}

class Task {
}

class Client {
  constructor(address) {
    this.connection = null;
    this.address = address;

    this.contexts = new Container();
    this.tasks = new Container();
    this.mounts = new Container();
  }

  async connect() {
    this.connection = new Connection(new Websocket(this.address), true);
    this.connection.onClose.add(() => {
      this.contexts.clear();
      this.tasks.clear();
      this.mounts.clear();
    });
  }

  disconnect() {
    return new Promise((resolve, reject) => {
      if (this.connection.isConnected()) {
        this.connection.onClose.add(() => {
          resolve();
        });
      } else {
        resolve();
      }
    }
  }

  // will block until the client is disconnected
  async run() {
  }

  query() {
    return new ClientQuery(this);
  }
}

// the query API
// allows for clients to build query objects and receive deltas
// when objects match those queries/do not match those queries

// that way resources can be referenced (i.e contexts/nodes) even when they are not yet available
// due to i.e the client losing connection

// you call update() on a query to feed in a resource
// so for a NamespaceQuery update() will

// represents a node query
class NodeQuery {
  constructor(node) {
  }
  update(node) {
  }
}

class ContextQuery {
}

class ContextsQuery {
}

class TaskQuery {
}

class TasksQuery {
}


//
class ClientQuery {
  constructor(client) {
    this.client = client;
    this.onChange = new Signal();
  }

  tasks() {
    var q = new TasksQuery(this.client ? this.client.tasks : null);
    // weak add so this query can be cleaned up automagically!
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.tasks : null));
  }

  contexts() {
    var q = new ContextsQuery(this.client ? this.client.contexts : null);
    // weak add so this query can be cleaned up automagically!
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.contexts : null));
  }

  mounts() {
    var q = new MountsQuery(this.client ? this.client.mounts : null);
    this.onChange.addWeak(q, (query, client) => query.update(client ? client.mounts : null));
  }

  update(client) {
    this.client = client;
    this.onChange.dispatch(client);
  }
}
