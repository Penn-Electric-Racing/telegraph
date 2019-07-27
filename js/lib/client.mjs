import r from 'rethinkdb'

import {Node, Group, Action, Variable, Stream} from './tree.mjs'

export class Client {
  constructor() {
    this._conn = null;
    this._node_key_map = new WeakMap();
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
      await this.remove(name);
      await this.add(name, tree);
    }
  }

  async get(name) {
  }

  // private API

  // this function will take a new local node
  // and map it into a server database, so that any child add/removes
  // done locally will be forwarded to the server (again using _map_local)
  // additionally, any subscriptions other poeple make or actions they
  // trigger will be forwarded to the local node
  async _map_local(name, node) {
    let parent_key = node.parent == null ? null : this._node_key_map[node.parent];
    let obj = { name : node.name, pretty : node.pretty, desc: node.desc, parent: parent_key };
    if (node instanceof Group) {
      obj.type = 'group';
      obj.schema = node.schema;
      obj.version = node.version;
    } else if (node instanceof Variable) {
      obj.type = 'variable';
      obj.data_type = 'none';
    } else if (node instanceof Action) {
      obj.type = 'action';
      obj.arg_type = 'none';
      obj.ret_type = 'none';
    } else if (node instanceof Stream) {
      obj.type = 'stream';
    }

    // do the insertion
    let result = await r.db(name).table('nodes').insert(obj).run(this._conn);
    let key = result.generated_keys[0];

    this._node_key_map[node] = key;

    // Now bind all the changes properly
    if (node instanceof Group) {
      // insert all of the children
      let promises = node.children.map(n => this._map_local(name, n));

      // add a listener to the Group to see when requests are made for
      // children to be added/removed and send additional map requests
      node.on_add_child.add(child => {
        this._map_local(name, child).then(function () {});
      });
      node.on_remove_child.add(child => {
        this._unmap_local(name, child).then(function() {});
      });
      await Promise.all(promises);
    } else if (node instanceof Variable) {
      // get a rethinkdb feed and let us know 
      // if someone subscribes to this variable
    } else if (node instanceof Action) {
    } else if (node instanceof Stream) {
    }
  }
}
