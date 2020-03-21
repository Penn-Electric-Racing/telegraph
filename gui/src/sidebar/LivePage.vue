<template>
  <div class="live-view">
    <TreeView class="live-tree" :treeQuery="liveNodesQuery" placeholder="No Connection"/>
    <div class="connection-selectors">
      <ComboBox :options="ports"/>
      <ComboBox :options="bauds"/>
      <Button :text="this.live ? 'Disconnect' : 'Connect'" @click="this.live ? disconnect() : connect()"/>
    </div>
  </div>
</template>

<script>
import TreeView from './TreeView.vue'

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
      live: null // the live context
    }
  },
  created() {
  },
  computed: {
    liveContextQuery() {
      return this.nsQuery.contexts.unwrap(); //filter(c => c.name == 'live' ).unwrap();
    },
    liveNodesQuery() {
      return this.liveContextQuery.fetch();
    }
  },
  methods: {
    async connect() {
    },
    async disconnect() {
    },
  },
  created() {
    if (this.liveContextQuery) {
      this.live = this.liveContextQuery.current;
      this.liveContextQuery.updated.add((newContext) => { this.live = newContext; });
    }
  },
  watch: {
    liveContext() {
      this.live = this.liveContextQuery.current;
      this.liveContextQuery.updated.add((newContext) => { this.live = newContext; });
    }
  }
}
</script>

<style scoped>
.live-view {
  height: 100%;
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: stretch;
}
.live-tree {
  height: 100%;
  width: 100%;
}
.connection-selector {
  width: 100%;
}
</style>
