#!/usr/bin/node --experimental-modules

import {Client, ClientError} from '../lib/client.mjs'
import {Tree, Node, Group, Variable, Action} from '../lib/tree.mjs'


function sleep(ms) {
  return new Promise(resolve => setTimeout(resolve, ms));
}

(async () => {
  try {
    var c = new Client();
    c.contextAdded.add((ctx) => { console.log('added: ' + ctx.getName()); })
    c.contextRemoved.add((ctx) => { console.log('removed: ' + ctx.getName()); })
    await c.connect('0.0.0.0:8000');

    // list the contexts
    console.log("Contexts:");
    for (let ctx of c.getContexts()) {
      console.log(ctx.getName() + ":");
      var tree = await ctx.get();
      console.log(tree ? tree.toString() : 'no tree');
      console.log("----------");
      tree.dispose();
    }
  } catch (e) {
    if (e instanceof ClientError) console.log(e.message);
    else throw e;
  }

  var ctx = c.getContexts()[0];
  console.log(tree ? tree.toString() : 'no tree!');
  if (tree) {
    var v = tree.find('/foo');
    var a = tree.find('/bar');

    var result = await a.call(false);

    console.log('called action, got: ' + result);

    var sub = v.subscribe(0, 0);
    sub.addHandler((ts, dp) => {
      console.log('got value: ' + dp);
    });
  }
})();
