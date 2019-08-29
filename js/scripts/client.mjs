#!/usr/bin/node --experimental-modules

import {Client} from '../lib/client.mjs'
import {Tree, Node, Group, Variable, Action} from '../lib/tree.mjs'


function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

(async () => {
  var c = new Client();
  c.contextAdded.add((ctx) => { console.log('added: ' + ctx.getName()); })
  c.contextRemoved.add((ctx) => { console.log('removed: ' + ctx.getName()); })
  await c.connect('0.0.0.0:8000');

  var ctx = c.getContexts()[0];
  var tree = await ctx.get();
  console.log(tree ? tree.toString() : 'no tree!');
  if (tree) {
    var v = tree.find('/b');
    var a = tree.find('/a/c');

    var result = await a.call(5);

    console.log('a');

    var sub = v.subscribe(0, 0);
    sub.addHandler((ts, dp) => { 
      console.log('got value: ' + dp); 
      a.call(dp).then((ret) => {
        console.log('called action, got: ' + ret);
      });
    });
  }
})();
