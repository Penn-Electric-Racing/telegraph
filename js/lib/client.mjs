import {Tree, Node, Group, Action, Variable, Type} from './tree.mjs'
import {Context} from './context.mjs'

import {default as grpc} from 'grpc'
import {default as protoLoader} from '@grpc/proto-loader'
import * as path  from 'path'

var PROTO_PATH = path.resolve('../proto/api.proto');

var packageDefinition = protoLoader.loadSync(PROTO_PATH, {keepCase: true, longs: String, enums: String, defaults: true, oneofs: true});
var server_proto = grpc.loadPackageDefinition(packageDefinition);


// Suggested options for similarity to existing grpc.load behavior
var packageDefinition = protoLoader.loadSync(
    PROTO_PATH,
    {keepCase: true,
     longs: String,
     enums: String,
     defaults: true,
     oneofs: true
    });
var protoDescriptor = grpc.loadPackageDefinition(packageDefinition);

var api = protoDescriptor.libcom;

function unpackNode(nodeIdMap, idNodeMap, obj) {
  var parent = null;
  var node = null;
  var id = -1;
  if (obj.id) {
    return idNodeMap.get(obj.id);
  } else if (obj.group) {
    let g = obj.group;
    parent = idNodeMap.get(g.parent);
    node = new Group(g.name, g.pretty, g.desc, g.schema, g.version);
    id = g.id;
  } else if (obj.var) {
    let v = obj.var;
    parent = idNodeMap.get(v.parent);
    node = new Variable(v.name, v.pretty, v.desc, Type.unpack(v.data_type));
    id = v.id;
  } else if (obj.action) {
    let a = obj.action;
    parent = idNodeMap.get(a.parent);
    node = new Action(a.name, a.pretty, a.desc, Type.unpack(a.arg_type), Type.unpack(a.ret_type));
    id = a.id;
  }

  nodeIdMap.set(node, id);
  idNodeMap.set(id, node);

  if (parent) parent.addChild(node);
  return [node, id];
}

export class Client {
  constructor() {
    this._client = null;
    this._varSource = null;

    this._idNodeMap = new Map();
    this._nodeIdMap = new Map();

    this._contexts = [];
    this._contextStream = null;
  }

  getConnection() { return this._client; }

  getContexts() {
    return this._contexts;
  }

  connect(binding) {
    return new Promise(function(resolve, reject) {
      this._client = new api.ContextManager(binding, grpc.credentials.createInsecure());

      this._contextStream = this._client.streamContexts();
      this._contextStream.on('data', function(delta) {
        var ctx = new Context(delta.context.name);
        ctx._setTree(this._fetchTree.bind(this, ctx, delta.context.id));

        switch(delta.type) {
          case 'INITIAL':
            this._contexts.push(ctx);
            break;
          case 'ADDED':
            this._contexts.push(ctx);
            break;
          case 'REMOVED':
            var i = 0;
            for (; i < this._contexts.length; i++) {
              if (this._contexts[i].id == delta.context.id) {
                this._contexts[i].dispose();
                this._contexts.splice(i, 1);
                break;
              }
            }
            break;
          case 'INITIALIZED':
            resolve();
            break;
        }
      }.bind(this));
      this._contextStream.on('end', function(end) {});
      this._contextStream.on('cancelled', function(end) {});
      this._contextStream.on('error', function(err) {});
      this._contextStream.on('status', function(status) {
      });
    }.bind(this));
  }

  disconnect() {
    return new Promise(function(resolve, reject) {
      this._contextStream.cancel();
      this._contextStream = null;

      // dispose of all the contexts and clear the context list
      for (let c of this._contexts) {
        c.dispose();
      }
      this._contexts = [];

      this._nodeIdMap.clear();
      this._idNodeMap.clear();
      if (this._client) this._client.close();
      this._client = null;
      resolve();
    }.bind(this));
  }

  async _fetchTree(ctx, ctxID) {
    return new Promise((resolve, reject) => {
      // Now we call StreamTree to get the tree
      var treeStream = this._client.streamTree({tree_id: ctxID});

      // add a disposer so that when context.dispose() is called
      // the stream gets cancelled
      ctx.onDispose.add(() => {
        if (ctx._tree) {
          for (let node of ctx._tree.nodes()) {
            if (this._nodeIdMap.has(node)) {
              let id = this._nodeIdMap.get(node);
              this._nodeIdMap.remove(node);
              this._idNodeMap.remove(id);
            }
          }
          ctx._tree = null;
        }
        treeStream.cancel();
      });

      var root = null;
      treeStream.on('data', (delta) => {
        switch(delta.type) {
          case 'ADDED':
          case 'INITIAL':
            let [added, addedId] = unpackNode(this._nodeIdMap, this._idNodeMap, delta);
            if (!added.getParent()) root = added;

            // if we have a variable, listen for new subscriptions
            if (added instanceof Variable) {
              added.onSubscribe.add((sub) => {
                var stream = this._client.subscribe({var_id: addedId, 
                                        min_interval: sub.getMinInterval(),
                                        max_interval: sub.getMaxInterval()});
                stream.on('data', (data) => {
                  sub.data(data.timestamp, sub.getType().unpackValue(data.value));
                });
                // if the server has ended the stream, 
                // destroy the subscriber
                stream.on('end', (e) => {
                  sub.cancel();
                });
                stream.on('error', (err) => {});
                // if the sub has been canceled, cancel the stream
                sub.onCancel.add(() => stream.cancel());
              });
            }
            // if we have an action, listen for action requests
            if (added instanceof Action) {
              added.setActor((arg) => {
                return new Promise((resolve, reject) => {
                  this._client.performAction(
                    {action_id: addedId, argument: added.getArgType().packValue(arg)}, 
                    function(err, result) {
                      if (result) resolve(added.getRetType().unpackValue(result));
                    });
                });
              });
            }
            break;
          case 'REMOVED':
            let [removed, removedId] = unpackNode(this._nodeIdMap, this._idNodeMap, delta);
            if (node.getParent()) node.getParent().removeChild(removed);
            this._nodeIdMap.remove(removed);
            this._idNodeMap.remove(removedId);
            break;
          case 'INITIALIZED': {
            let tree = new Tree(root);
            root = null;
            resolve(tree);
          } break;
        }
      });

      treeStream.on('end', function(end) {});
      treeStream.on('error', function(error) {});
    });
  }
}
