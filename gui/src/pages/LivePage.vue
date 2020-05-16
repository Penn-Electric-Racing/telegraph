<template>
  <div class="live-page">
    <TreeView class="live-tree" :treeQuery="liveNodesQuery" placeholder="No Connection"/>
    <div class="connection-selectors">
      <ComboBox :options="ports"/>
      <ComboBox :options="bauds"/>
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
      portsStream: null,
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
    devicesCollection() {
      if (this.devicesCollection) this.devicesCollection.unbind(this.devices);
      this.devices = [];
      var collection = this.nsQuery.contexts.filter(x => x.type == 'device').collect();
      collection.bind(this.devices)
      return collection;
    }
  },
  methods: {
    liveContextUpdated(live) {
      this.live = live;
    },
    scannerUpdated(scanner) {
      if (!scanner && this.nsQuery.current) {
        // request to create a scanner
        var ns = this.nsQuery.current;
        ns.createComponent('scanner', 'device_scanner', {}, {});
      }
      if (this.portsStream) {
        this.portsStream.close();
        this.portsStream = null;
      }
      if (scanner) {
        (async() => {
          this.portsStream = await scanner.request(null);
          this.portsStream.received.add((x) => this.ports = x);
          this.portsStream.start();
        })();
      }
      this.scanner = scanner;
    },
    async connect() {
    },
    async disconnect() {
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
    }
  },
  created() {
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
.connection-selector {
  width: 100%;
}
</style>
