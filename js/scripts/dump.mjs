#!/usr/bin/node --experimental-modules

import {Client} from '../index.mjs'
import {Tree, Node, Group, Variable, Action, Stream} from '../index.mjs'

var c = new Client();

(async () => {
  await c.connect("localhost", 28015);
  console.log('connected');
  let t = await c.get("live");
  if (t == null) {
    console.log('could not find tree');
  } else {
    console.log(t.toString());
  }
})();
console.log('done');
