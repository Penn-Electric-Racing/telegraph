#!/usr/bin/node --experimental-modules

import {Client} from '../index.mjs'
import {Tree, Node, Group, Variable, Action, Stream, Type} from '../index.mjs'

var c = new Client();

var g = new Group("root", "Group", "A Group", "test/group", 1);
var v = new Variable("var", "Variable", "A test variable", Type.FLOAT);

class TestSubscription {
  constructor(source, node) {
    this._source = source;
    this._node = node;
    this._listeners = [];
  }
  unsubscribe() {
    this._source._unsubscribe(this);
  }

  addHandler(handler) {
    this._listeners.push(handler);
  }

  _handleValue(ts, value) {
    for (let l of this._listeners) {
      l(ts, value);
    }
  }
}

/** 
 * Sources only need to have a subscribe() and return subscription objects that have addHandler(), removeHandler(), and cancel()
 * function
 */
class TestSource {
  constructor() {
    this.subscriptions = new Map();
  }
  publish(node, ts, value) {
    if (!this.subscriptions.has(node)) return;
    for (let s of this.subscriptions.get(node)) {
      s._handlevalue(ts, value);
    }
  }

  subscribe(node, min_interval, max_interval) {
    var s = new TestSubscription(this);
    if (!this.subscriptions.has(node)) this.subscriptions.put(node, []);
    this.subscriptions.get(node).push(s);
    return s;
  }

  _unsubscribe(subscription) {
    var n = subscription._node;
    if (!this.subscriptions.has(n)) return;
    var l = this.subscriptions.get(n);
    var idx = l.indexOf(subscription);
    if (idx >= 0) l.splice(idx);
    if (l.length == 0) this.subscriptions.remove(n);
  }
}

var source = new TestSource();
v.setSource(source);

g.add_child(v);

var t = new Tree(g);

function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}


(async () => {
  await c.connect("localhost", 28015);
  console.log('connected');
  await c.replace("live", t);

  let i = 0;
  while (true) {
    await sleep(2000);
    source
    i++;
  }
})();
