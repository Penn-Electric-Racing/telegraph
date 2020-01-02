import { Group, Variable, Type, Relay, LocalNamespace, 
         LocalDevice, LocalContainer, Context } from './index.mjs'

(async () => {
  var ns = new LocalNamespace();
  var relay = new Relay(ns);
  relay.bind(8081);

  var variable = new Variable('var', 'Variable', '', Type.BOOL);
  var tree = new Group('root', 'Root', 'The root node.', 'root', 1.0, [variable]);

  // create a new device in the namespace
  // with this tree
  var dev = new LocalDevice(ns, 'dev', '/dev/ttyUSB0', [100, 200], tree);

  // create a new container in the namespace
  // with this tree
  var container = new LocalContainer(ns, 'live', tree.clone());

  // mount the device on the container!
  await container.mount(dev);
})();
