<template>
  <div class="live-view">
    <TreeView class="live-tree" :tree="liveTree" placeholder="No Connection"/>
    <div class="connection-selectors">
        <ComboBox :options="ports" title="Ports"/>
        <ComboBox :options="bauds" title="Bauds"/>
        <Button :text="connectionText" @click="connect()" v-show="liveContext"/>
    </div>
  </div>
</template>

<script>
import TreeView from './TreeView.vue'

import ComboBox from '../components/ComboBox.vue'
import Button from '../components/Button.vue'
import { ContextsRetriever, ContextRetriever, Namespace } from 'telegraph'

export default {
  name: 'LivePage',
  components: {TreeView, ComboBox, Button},
  props: {
    ns: Namespace
  },
  data: function() {
    return {
      ports : ['Test'],
      bauds : ['Auto', 10, 11, 123],
      devices: [],
      live: null,
      liveTree: null,
      devicesRetriever: null,
      liveRetriever: null,
      liveContext: true
    }
  },
  computed: {
    connectionText() {
      return this.liveContext ? 'Connected' : 'Disconnected'
    },
  },
  methods: {
    connect() {
      liveContext = false
    },
    async disconnect() {
      this.liveContext = false
    },
    namespaceChanged(ns) {
      this.devicesRetriever.namespaceChanged(ns);
      this.liveRetriever.namespaceChanged(ns);
    }
  },
  watch: {
    ns(val) { 
      this.namespaceChanged(val) 
    },
    live(ctx) {
      if (ctx) ctx.fetch().then((t) => this.liveTree = t);
      else this.liveTree = null;
    }
  },
  created() {
    this.devicesRetriever = new ContextsRetriever({by_type:'device'}, this.devices),
    this.liveRetriever = new ContextRetriever({by_name:'live'}, (ctx) => this.live = ctx)
    this.namespaceChanged(this.ns);
  },
  destroyed() {
    this.devicesRetriever.stop();
    this.liveRetriever.stop();
    this.devicesRetriever = null;
    this.liveRetriever = null;
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
.connection-selectors {
  width: 100%;
  display: flex;
  overflow: visible;
}
.connection-selectors > *{
  width: 100%;
  height: 30px;
}


</style>
