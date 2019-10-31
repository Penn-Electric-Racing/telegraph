#!/usr/bin/env node --experimental-modules

import {Server} from '../lib/server.mjs'
import {Context} from '../lib/context.mjs'
import {Tree, Node, Group, Variable, Type, Action} from '../lib/tree.mjs'

var server = new Server();
server.start('0.0.0.0:8000');

var g = new Group("group", "The Group", "A Group", "testbar/group", 1);
var a = new Group("a", "Group Two", "Another group", "g/test", 1);
var b = new Variable('b', 'The Variable', 'Testing Variable', Type.BOOL);
var c = new Action('c', 'The Action', 'Testing Action', Type.BOOL, Type.INT8);

g.addChild(a);
g.addChild(b);
a.addChild(c);

c.setActor((arg) => arg ? 10 : 20);

var t = new Tree(g);

var ctx = new Context('live', t);
server.addContext(ctx);

function sleep(milli) {
  return new Promise(resolve => setTimeout(resolve, milli));
}

var subscribers = [];
b.onSubscribe.add((s) => { subscribers.push(s); });

(async () => {
  for (let i = 0; true; i++) {
    await sleep(1000);
    for (let s of subscribers) {
      s.data(i, i % 2 == 0);
    }
  }
})();
