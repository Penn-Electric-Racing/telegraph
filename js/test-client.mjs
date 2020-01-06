import { Relay, LocalNamespace, Context } from './index.mjs'
import protobuf from 'protobufjs'

(async () => {
  try {
    var proto = await protobuf.load('../proto/api.proto')
  } catch (e) {
    console.log(e);
  }
  var ns = new LocalNamespace();
  var relay = new Relay(ns, proto);

  // connect (gives us a namespace to play around with)
  var fns = await relay.connect('ws://localhost:8081/');
  console.log('connected!');

  // fetch the live contexts from this namespace
  var ctxs = await fns.contexts({by_name:'live'});
  var ctx = [...ctxs.all][0];
  var tree = await ctx.fetch();
  console.log(tree.toString());
})()

