import { Relay, LocalNamespace, Context } from './index.mjs'

var ns = new LocalNamespace();
var relay = new Relay(ns);
relay.bind(8081);
