<template>
  <div class="live-view">
    <TreeView class="live-tree" :tree="liveTree" placeholder="No Connection"/>
    <div class="connection-selectors">
      <ComboBox :options="ports"/>
      <ComboBox :options="bauds"/>
      <Button :text="connectionText"/>
    </div>
  </div>
</template>

<script>
import TreeView from './TreeView.vue'

import ComboBox from '../components/ComboBox.vue'
import Button from '../components/Button.vue'
import { Namespace } from 'telegraph'

export default {
  name: 'LivePage',
  components: {TreeView, ComboBox, Button},
  props: {
    ns: Namespace
  },
  data: function() {
    return {
      ports : ['/dev/ttyUSB0'],
      bauds : ['Auto', 10, 11, 123],
      devicesFeed: null,
      liveFeed: null,
      liveContext: null,
      liveTree: null
    }
  },
  computed: {
    connectionText() {
      return this.liveContext ? 'Disconnect' : 'Connect'
    }
  },
  methods: {
    async namespaceChanged(ns) {
      if (this.liveFeed) {
        this.liveFeed.close();
        this.liveFeed = null;
        this.liveContext = null;
        this.liveTree = null;
      }
      if (this.devicesFeed) {
        this.devicesFeed.close();
        this.devciesFeed = null;

        this.ports = [];
        this.bauds = [];
      }
      if (ns) {
        this.devicesFeed = await ns.contexts({by_type: 'device'});
        for (let c of this.devicesFeed.all) {
          this.ports.push(c.getInfo().port);
        }
        this.devicesFeed.added.add(
          c => this.ports.push(c.getInfo().port));
        this.devicesFeed.removed.add(
          c => this.ports.splice(this.ports.indexOf(c.getInfo().port), 1));

        // setup the query for the live feed
        this.liveFeed = await ns.contexts({by_name:'live'});
        // check if 
        var ctxs = [...this.liveFeed.all];
        if (ctxs.length > 0) {
          this.liveContext = ctxs[0];
          this.liveTree = await this.liveContext.fetch();
        }
        this.liveFeed.added.add((ctx) => { 
          if (!this.liveContext) {
            this.liveContext = ctx;
            this.liveContext.fetch().then(tree => this.livetree = tree);
          }
        });
        this.liveFeed.removed.add((ctx) => {
          if (this.liveContext == ctx) {
            this.liveContext = null;
            this.liveTree = null;
          }
        });
      }
    }
  },
  watch: {
    ns(val) { this.namespaceChanged(val) },
  },
  created() {
    this.namespaceChanged(this.ns);
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
