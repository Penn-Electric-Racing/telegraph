<template>
  <div class="live-page">
    <TreeView class="live-tree" :treeQuery="liveNodesQuery" placeholder="No Connection"/>
    <div class="connection-selectors">
      <ComboBox :options="ports"/>
      <ComboBox :options="bauds"/>
      <Button :text="this.live ? 'Disconnect' : 'Connect'" @click="this.live ? disconnect() : connect()"/>
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
      ports : [],
      bauds : ['Auto', 10, 11, 123],
      devices: [],
      devicesStream: null,
      live: null // the live context
    }
  },
  computed: {
    liveContextQuery() {
      return this.nsQuery.contexts.unwrap(); //filter(c => c.name == 'live' ).unwrap();
    },
    liveNodesQuery() {
      return this.liveContextQuery.fetch();
    },
    scannerQuery() {
      return this.nsQuery.components.extract(x => x.type == 'device_scanner');
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
