#!/usr/bin/node --experimental-modules

import {Client} from '../index.mjs'
import {Tree, Node, Group, Variable, Action, Stream} from '../index.mjs'

var c = new Client();

var g = new Group("group", "Group", "A Group", "test/group", 1);
var a = new Group("a", "bar", "flub", "g/test", 1);
var b = new Group("b", "bar", "flub", "g/test", 1);
g.add_child(a);
var t = new Tree(g);

(async () => {
  await c.connect("localhost", 28015);
  console.log('connected');
  await c.replace("live", t);
  console.log('listening');
  g.add_child(b);
})();
console.log('done');
