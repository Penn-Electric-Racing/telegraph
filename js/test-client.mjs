import { Client } from './index.mjs'

(async () => {
  let ns = new Client();

  try {
    await ns.connect('ws://localhost:8081/');
    var devs = ns.contexts.filter(c => c.type == 'device');
    if (devs.empty) {
      await ns.createContext('live', 'device', { baud: 115200, port: '/dev/ttyACM0' });
    }
    // dev should now be here!
    let dev = devs.unwrap();
    let tree = await dev.fetch();
    console.log(tree.toString());
    await dev.destroy(); // close the device
  } catch (e) {
    console.log(e);
  } finally {
    await ns.disconnect();
  }
})()

