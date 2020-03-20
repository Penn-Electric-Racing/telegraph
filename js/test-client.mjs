import { Client } from './index.mjs'

(async () => {
  try {
    var ns = new Client();
    await ns.connect('ws://localhost:8081/');
  } catch (e) {
    console.log(e);
  }
})()

