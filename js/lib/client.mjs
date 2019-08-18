import r from 'rethinkdb'

import {Tree, Node, Group, Action, Variable, Type, Stream} from './tree.mjs'

// go through until the feed is ready
async function feed_initial(feed, func, obj=undefined) {
  let c = await feed.next();
  while (c != null) {
    if (!c.state) {
      if (obj != undefined) await func(obj, c);
      else await func(c);
    } else if (c.state == 'ready') break;
    c = await feed.next();
  }
}

function feed_tie(feed, obj, func) {
  let run = (async () => {
    while (true) {
      let change = await feed.next();
      func(obj, change);
    }
  })();
  // when the promise ends, cancel the changefeed
  run.then(function() {
    feed.close();
  });
}

async function feed_tie_with_initial(feed, obj, func) {
  await feed_initial(feed, func, obj);
  feed_tie(feed, obj, func);
}

// There exists one remote source per variable,
// and each RemoteSource creates a single subscription
// object in the rethinkdb database, 
// passing along any returned values
class RemoteSubscription {
  constructor(source, min_interval, max_interval) {
    this._source = source;
    this._listeners = [];
    // min and max interval here are fixed
    // once the subscription is made
    this._min_interval = min_interval;
    this._max_interval = max_interval;

    // start the subscription 
    // (will spawn an asynchronous task)
    this._start();
  }

  async _start() {
    var obj = {
      node : this._source._node_key,
      min_interval : this._min_interval,
      max_interval : this._max_interval
    };
    var res = await r.db(this.source._db_name).table('subscriptions')
                     .insert(obj).run(this._source._conn);
    console.log(res);
  }

  // every subscription needs a cancel, addHandler(), and removeHandler()
  cancel() {
    this._source._unsubscribe(this);
  }

  addHandler(handler) {
    this._listeners.push(handler);
  }
};

class RemoteSource {
  constructor(conn, db_name, node_key) {
    this._db_name = db_name;
    this._node_key = node_key;
    this._subscriptions = [];
    this._conn = conn;
  }

  // handles any changes to the min/max interval
  // or the subscription being added or deleted
  subscribe(node, min_interval, max_interval) {
    this._subscriptins.push(new RemoteSubscription(this, min_interval, 
                                                      max_interval));
  }
};

export class Client {
  constructor() {
    this._conn = null;
    this._node_key_map = new Map();
    this._key_node_map = new Map();
  }

  async connect(host, port) {
    return new Promise(resolve => {
      var connected = function(err, conn) {
        if (err) throw err;
        this._conn = conn;
        resolve();
      }.bind(this);
      r.connect({host: host, port: port}, connected);
    });
  }

  async add(name, tree) {
    if (await r.dbList().contains(name).run(this._conn)) {
      throw new Error("Tree " + name + " already exists");
    }
    await r.dbCreate(name).run(this._conn);
    var db = r.db(name);
    var r1 = db.tableCreate("nodes").run(this._conn);
    var r2 = db.tableCreate("subscriptions").run(this._conn);
    var r3 = db.tableCreate("actions").run(this._conn);
    var r4 = db.tableCreate("log").run(this._conn);
    await Promise.all([r1, r2, r3, r4]);
    await db.table("nodes").indexCreate("parent").run(this._conn);
    await db.table("subscriptions").indexCreate("node").run(this._conn);
    await this._map_local(name, tree.root, null);
  }

  async remove(name) {
    if (!await r.dbList().contains(name).run(this._conn)) {
      throw new Error("Tree " + name + " does not exist");
    }
    await r.dbDrop(name).run(this._conn);
  }

  async replace(name, tree) {
    if (!await r.dbList().contains(name).run(this._conn)) {
      await this.add(name, tree);
    } else {
      function clear_table(table_name, conn) {
        return r.branch(
          r.db(name).tableList().contains(table_name),
          r.db(name).table(table_name).delete(),
          r.db(name).tableCreate(table_name)).run(conn);
      }
      let q1 = clear_table('nodes', this._conn);
      let q2 = clear_table('subscriptions', this._conn);
      let q3 = clear_table('actions', this._conn);
      let q4 = clear_table('log', this._conn);
      await Promise.all([q1, q2, q3, q4]);
      // create an index for the parent if it does not already exist
      await r.branch(
        r.db(name).table("nodes").indexList().contains("parent"),
        {}, 
        r.db(name).table('nodes').indexCreate("parent")).run(this._conn);
      await r.branch(
        r.db(name).table("subscriptions").indexList().contains("node"),
        {}, 
        r.db(name).table('subscriptions').indexCreate("node")).run(this._conn);
      // map the locals in
      await this._map_local(name, tree.root, null);
    }
  }

  async get(name) {
    if (!await r.dbList().contains(name).run(this._conn)) {
      return null;
    } 
    // search by nodes without a parent
    let roots = await r.db(name).table('nodes').getAll('', {index: 'parent'}).run(this._conn);

    let roots_array = await roots.toArray();
    if (roots_array.length != 1) {
      throw new Error("Tree " + name + " does not have a single root node");
    }
    return new Tree(await this._map_remote(name, roots_array[0].id));
  }

  // private API

  // this function will take a new local node
  // and map it into a server database, so that any child add/removes
  // done locally will be forwarded to the server (again using _map_local)
  // additionally, any subscriptions other poeple make or actions they
  // trigger will be forwarded to the local node
  async _map_local(tree_name, node) {
    if (this._node_key_map.has(node)) return;
    let parent_key = node.parent == null ? '' : this._node_key_map.get(node.parent);
    let obj = { name : node.name, pretty : node.pretty, 
                desc: node.desc, parent: parent_key };
    if (node instanceof Group) {
      obj.type = 'group';
      obj.schema = node.schema;
      obj.version = node.version;
    } else if (node instanceof Variable) {
      obj.type = 'variable';
      obj.data_type = node.getType().pack();
    } else if (node instanceof Action) {
      obj.type = 'action';
      obj.arg_type = 'none';
      obj.ret_type = 'none';
    } else if (node instanceof Stream) {
      obj.type = 'stream';
    }

    // do the insertion
    let result = await r.db(tree_name).table('nodes')
                        .insert(obj).run(this._conn);
    let key = result.generated_keys[0];

    this._node_key_map.set(node, key);
    this._key_node_map.set(key, node);

    node.on_parent_changed.add(parent_node => {
      if (parent_node) { // the node's parent has just changed
        let parent_key = this._node_key_map.get(parent_node);
        r.db(tree_name).table('nodes').get(key)
            .update({parent: parent_key}).run(this._conn); 
      } else { // this node has been removed from the tree completely
        this._unmap_local(tree_name, node);
      }
    });

    // Now bind all the changes properly
    if (node instanceof Group) {
      // insert all of the children
      let promises = node.children.map(n => this._map_local(tree_name, n));

      node.on_add_child.add(child => {
        // just map it, the on_parent_changed will
        // take care of the rest

        // and it will be unammped if removed
        // from the tree
        this._map_local(tree_name, child);
      });
      await Promise.all(promises);
    } else if (node instanceof Variable) {
      // get a rethinkdb feed and let us know 
      // if someone subscribes to this variable
      let subscribers =
        await r.db(tree_name).table('subscriptions')
                .getAll(key, {index:'node'}).changes({includeStates: true, 
                                                      includeInitial: true})
                .run(this._conn);

      await feed_tie_with_initial(subscribers, node, 
        async (n, change) => {
          console.log(change);
        });
    } else if (node instanceof Action) {
    } else if (node instanceof Stream) {
    }
  }

  async _map_remote(tree_name, node_key) {
    if (node_key == '') return null;
    if (this._key_node_map.has(node_key)) {
      return this._key_node_map.get(node_key);
    }

    let nodes = r.db(tree_name).table('nodes');

    let obj = await nodes.get(node_key).run(this._conn);
    if (obj == null) return;

    let name = obj.name;
    let pretty = obj.pretty;
    let desc = obj.desc;

    var node = null;

    switch (obj.type) {
      case 'group':
        node = new Group(name, pretty, desc, obj.schema, obj.version);
        break;
      case 'variable':
        node = new Variable(name, pretty, desc, Type.unpack(obj.data_type));
        break;
    }

    this._node_key_map.set(node, node_key);
    this._key_node_map.set(node_key, node);

    if (node instanceof Group) {
      let children = await r.db(tree_name).table('nodes').getAll(node_key, {'index': 'parent'}).changes({includeStates: true, includeInitial: true}).run(this._conn);

      await feed_tie_with_initial(children, node, async (n, change) => {
        if (change.new_val) {
          var child_key = change.new_val.id;
          var child = await this._map_remote(tree_name, child_key);
          n.add_child(child);
        }
      });

    } else if (node instanceof Variable) {
      node.setSource(function(n, min_interval, max_interval) {
        // when someone subscribes to this we want to create an entry
        // in the subscriptions database
      });
    }
    return node;
  }
}
