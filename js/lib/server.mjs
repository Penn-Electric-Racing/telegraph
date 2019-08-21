import {Tree, Node, Group, Action, Variable, Type} from './tree.mjs'

import {default as grpc} from 'grpc'
import {default as protoLoader} from '@grpc/proto-loader'
import * as path  from 'path'

var PROTO_PATH = path.resolve('../proto/api.proto');

// Suggested options for similarity to existing grpc.load behavior
var packageDefinition = protoLoader.loadSync(
    PROTO_PATH,
    {keepCase: true,
     longs: String,
     enums: String,
     defaults: true,
     oneofs: true
    });
var pkg_proto = grpc.loadPackageDefinition(packageDefinition);

var api = pkg_proto.libcom;

// utility packing functions

function packNode(id, pid, node) {
  let obj = {
    id: id, 
    parent: pid,
    name: node.getName(),
    pretty: node.getPretty(),
    desc: node.getDesc()
  };

  if (node instanceof Group) {
    obj.schema = node.getSchema();
    obj.version = node.getVersion();
    return {group: obj, node: 'group'}
  } else if (node instanceof Variable) {
    obj.data_type = node.getType().pack();
    return {var: obj, node: 'var'}
  } else if (node instanceof Action) {
    obj.arg_type = node.getArgType().pack();
    obj.ret_type = node.getRetType().pack();
    return {action: obj, node: 'action'};
  }
}


export class Server {
  constructor() {
    this._server = new grpc.Server();
    this._server.addService(api.ContextManager.service, 
      {StreamContexts: this._streamContexts.bind(this),
        StreamTree : this._streamTree.bind(this),
        Subscribe : this._subscribe.bind(this),
        PerformAction : this._performAction.bind(this)});
    // context id counting:
    this._ctxIdCounter = 0;
    this._ctxIdMap = new Map(); // id -> ctx
    this._idCtxMap = new Map(); // ctx -> id

    this._ctxsListeners = []; // listeners for ctx add/remove

    // map from ctx_id to add/remove listeners 
    // for updating the node ids
    this._ctxIDListeners = new Map();

    // node id counting
    this._idCounter = 0;
    this._idNodeMap = new Map(); // id -> node
    this._nodeIdMap = new Map(); // node -> id
  }

  addContext(context) {
    if (!context) 
      throw new Error("Cannot add null context");
    if (this._ctxIdMap.has(context))
      throw new Error("Context with this ID already exists");

    let id = this._ctxIdCounter++;
    this._ctxIdMap.set(context, id);
    this._idCtxMap.set(id, context);

    // get the tree from the context
    // (potentially this tree could be coming from another client!)
    context.get().then((tree) => {
      // if it has been removed in the meanwhile
      if (!this._ctxIdMap.has(context)) return;

      // setup id handlers as listeners
      // before the contextsListeners are notified
      var added = (node) => {
        let id = this._idCounter++;
        this._nodeIdMap.set(node, id);
        this._idNodeMap.set(id, node);
      };
      var removed = (node) => {
        let id = this._nodeIdMap.get(node);
        this._nodeIdMap.remove(node);
        this._idNodeMap.remove(id);
      };

      tree.onNodeAdded.add(added);
      tree.onNodeRemoved.add(removed);

      for (let n of tree.nodes()) {
        added(n);
      }

      this._ctxIDListeners.set(context, {added: added, removed: removed});

      for (let l of this._ctxsListeners) {
        l.added(context);
      }
    });
  }

  removeContext(context) {
    if (!this._ctxIdMap.has(context)) throw new Error("No such context!");
    let id = this.ctxIdMap.get(context);
    this.ctxIdMap.remove(context);
    this.idCtxMap.remove(id);
    if (this._ctxIdListeners.has(context)) {
      let l = this._ctxIdListeners.get(context);
      this._ctxIdListeners.remove(context);
      context.get().then((tree) => {
        tree.onNodeRemoved.remove(l.removed);
        tree.onNodeAdded.remove(l.added);
        for (let listener of this._ctxsListeners) {
          listener.removed(context);
        }
      });
    }
  }

  start(bind) {
    this._server.bind(bind, grpc.ServerCredentials.createInsecure());
    this._server.start();
  }

  _streamContexts(call) {
    // call never ends! client must cancel
    for (let [id, c] of this._idCtxMap) {
      call.write({type: 'INITIAL', context: {id : id, name : c.getName() } });
    }
    call.write({type: 'INITIALIZED', context: {id: 0, name: ''}});

    var l = {
      added: function(c) {
        call.write({type: 'ADDED', context: {id: c.getID(), name: c.getName()} });
      },
      removed: function() {
        call.write({type: 'REMOVED', context: {id: c.getID(), name: c.getName()} });
      }
    };
    this._ctxsListeners.push(l);

    // when the client cancels the stream that means
    // that it no longer wants to listen for updates
    call.on('cancelled', function() {
      let i = this._ctxsListeners.indexOf(l);
      if (i >= 0) this._ctxsListeners.splice(i, 1);
      call.end();
    }.bind(this));
  }

  _streamTree(call) {
    var ctx = this._idCtxMap.get(call.request.tree_id);
    if (!ctx) {
      call.end();
      return;
    }
    ctx.get().then((tree) => {
      // send over the initial nodes
      for (let node of tree.nodes()) {
        if (!this._nodeIdMap.has(node)) throw new Error("No ID found for node");
        let id = this._nodeIdMap.get(node);
        let pid = node.getParent() ? this._nodeIdMap.get(node.getParent()) : -1;
        let obj = packNode(id, pid, node);
        call.write({type: 'INITIAL', ...obj});
      }
      call.write({type: 'INITIALIZED', id: 0 });

      var added = function(node) {
        if (!this._nodeIdMap.has(node)) throw new Error("No ID found for node");
        let id = this._nodeIdMap.get(node);
        let pid = node.getParent() ? this._nodeIdMap.get(node.getParent()) : -1;
        let obj = packNode(id, pid, node);
        call.write({type: 'ADDED', ...obj});
      }.bind(this);

      var removed = function(node) {
        let nodeId = idMap.get(node);
        if (nodeId >= 0) call.write({type: 'REMOVED', id: nodeId });
        idMap.remove(node);
      }.bind(this);

      // add/remove listeners
      tree.onNodeAdded.add(added);
      tree.onNodeRemoved.add(removed);
      call.on('cancelled', function() {
        tree.onNodeAdded.remove(added);
        tree.onNodeRemoved.remove(removed);
      });
    });
  }

  _subscribe(call) {
    let n = this._idNodeMap.get(call.request.var_id);
    if (!n) call.end();
    let sub = n.subscribe(call.request.min_interval, call.request.max_interval);
    sub.addHandler((ts, val) => {
      let obj = { timestamp: ts, value: sub.getType().packValue(val) };
      call.write(obj);
    });
    sub.onCancel.add(function() {
      call.end();
    });
    call.on('cancelled', function() {
      sub.cancel();
    });
  }

  _performAction(call, callback) {
    let n = this._idNodeMap.get(call.request.action_id);
    if (!n) throw new Error('Unable to find action for id: ' + call.request.action_id);
    let val = n.getArgType().unpackValue(call.request.argument);
    n.call(val).then((res) => 
      callback(null, res ? n.getRetType().packValue(res) : Type.NONE.packValue(res)));
  }
}
