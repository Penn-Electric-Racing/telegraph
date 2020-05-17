<template>
  <div class="live-page">
    <TreeView class="live-tree" :treeQuery="liveNodesQuery" placeholder="No Connection"/>
    <div class="connection-selectors">
      <div class="connection-combobox">
        <ComboBox :options="ports" 
                  :locked="devices.length > 0" 
                  placeholder="Ports..." v-model="selectedPort"/>
      </div>
      <div class="connection-combobox">
        <ComboBox :options="bauds" 
                :locked="devices.length > 0" 
                placeholder="Baud..." v-model="selectedBaud"/>
      </div>
      <Button :text="devices.length > 0 ? 'Disconnect' : 'Connect'" 
          @click="devices.length > 0 ? disconnect() : connect()"/>
    </div>
  </div>
</template>

<script>
import TreeView from '../views/TreeView.vue'

import ComboBox from '../components/ComboBox.vue'
import Button from '../components/Button.vue'
import { NamespaceQuery } from 'telegraph'

export default {
  name: 'LivePage',
  components: {TreeView, ComboBox, Button},
  props: {
    nsQuery: NamespaceQuery
  },
  data: function() {
    return {
      bauds : [115200, 9600],
      ports: [],
      selectedPort: null,
      selectedBaud: null,
      portsStream: null,
      devicesCollection: null,
      devices: [],
      live: null // the live context
    }
  },
  computed: {
    liveContextQuery() {
      return this.nsQuery.contexts.extract(x => x.name == 'live' && x.type == 'container');
    },
    liveNodesQuery() {
      return this.liveContextQuery.fetch();
    },
    scannerQuery() {
      return this.nsQuery.components.extract(x => x.type == 'device_scanner');
    },
  },
  methods: {
    liveContextUpdated(live) {
      this.live = live;
    },
    scannerUpdated(scanner) {
      this.ports = [];
      if (!scanner && this.nsQuery.current) {
        // request to create a scanner
        var ns = this.nsQuery.current;
        ns.createComponent('scanner', 'device_scanner', {}, {});
      }
      if (scanner) {
        (async() => {
          if (this.portsStream) {
            this.portsStream.close();
            this.portsStream = null;
          }
          this.portsStream = await scanner.request(null);
          this.portsStream.received.add((x) => this.ports = x);
          this.portsStream.start();
        })();
      }
      this.scanner = scanner;
    },
    nsQueryUpdated() {
      if (this.nsQuery) {
        this.devicesCollection = 
          this.nsQuery.contexts.filter(x => x.type == 'device').collect();
        this.devices = [];
        for (let c of this.devicesCollection) this.devices.push(c);
        this.devicesCollection.added.add(x => this.devices.push(x));
        this.devicesCollection.removed.add(x => this.devices.splice(this.devices.indexOf(x), 1));
      } else {
        this.devices = [];
        this.devicesCollection = null;
      }

    },
    async connect() {
      if (this.selectedPort == null || this.selectedBaud == null)
        return;
      if (this.nsQuery.current) {
        let ns = this.nsQuery.current;
        // create the device
        let device = await ns.createContext(this.selectedPort, 'device', 
                  {port: this.selectedPort, baud: this.selectedBaud}, {});
        if (!device) return;

        // find if there is a live context
        let live = ns.contexts
            .extract(x => x.type == 'container' && x.name == 'live');
        if (live == null) {
            // create a new context if one does not exist
            console.log('creating first!', device);
            live = await ns.createContext('live', 
                    'container', {}, {src: device});
        }
        if (!live) return;
        try {
          await live.mount(device);
        } catch (e) {
          // if mount fails, destroy
          // the live context, recreate
          // and remount
          await live.destroy();
          console.log('creating!');
          live = await ns.createContext('live', 'container', 
                                      {}, {src: device});
          await live.mount(device);
        }
      }
    },

    async disconnect() {
      if (!this.nsQuery || !this.nsQuery.current) return;
      let ns = this.nsQuery.current;
      let dc = [];
      for (let d of this.devices) dc.push(d);
      for (let d of dc) {
        await d.destroy();
      }
      // if no-one is using the live context anymore
      if (this.live && ns.mounts.filter(x => x.tgt == this.live).size == 0) {
        await this.live.destroy();
      }
    },
  },
  watch: {
    liveContextQuery(n, o) {
      if (o) {
        o.updated.remove(this.liveContextUpdated);
      }
      this.liveContextUpdated(this.liveContextQuery.current);
      this.liveContextQuery.updated.add(this.liveContextUpdated);
    },
    scannerQuery(n, o) {
      if (o) {
        o.updated.remove(this.scannerUpdated);
      }
      this.scannerUpdated(this.scannerQuery.current);
      this.scannerQuery.updated.add(this.scannerUpdated);
    },
    nsQuery() {
      this.nsQueryUpdated();
    },
  },
  created() {
      this.nsQueryUpdated();
      this.liveContextUpdated(this.liveContextQuery.current);
      this.liveContextQuery.updated.add(this.liveContextUpdated);

      this.scannerUpdated(this.scannerQuery.current);
      this.scannerQuery.updated.add(this.scannerUpdated);
  }
}
</script>

<style scoped>
.live-page {
  height: 100%;
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: stretch;
}
.live-tree {
  height: 100%;
  width: 100%;
  min-height: 0;
}
.connection-selectors {
  display: flex;
  flex-direction: row;
  align-items: stretch;
}
.connection-combobox {
  display: flex;
  flex: 1;
}
</style>
