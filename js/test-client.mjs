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
  relay.connect('ws://localhost:8081/');
})()

