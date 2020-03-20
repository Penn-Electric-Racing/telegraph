import { Client } from './index.mjs'

(async () => {
  let ns = new Client();

  try {
    await ns.connect('ws://localhost:8081/');
    let dev = await ns.createContext('live', 'device', { baud: 115200, port: '/dev/ttyACM0' });
  } catch (e) {
    console.log(e);
  } finally {
    await ns.disconnect();
  }
})()

