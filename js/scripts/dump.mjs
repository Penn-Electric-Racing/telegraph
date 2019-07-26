#!/usr/bin/node --experimental-modules

import {Client} from '../index.mjs'

var c = new Client();


(async () => {
  await c.connect("localhost", 28015);
})();
