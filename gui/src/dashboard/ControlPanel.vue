<template>
  <Panel @close="$emit('delete')" :header="header">
    <div class="controls">
      <Control :node="n" v-for="n in controllableNodes"/>
    </div>
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import Control from './Control.vue'
  import { Namespace, Action, Variable, ContextRetriever } from 'telegraph'
  export default {
    name: 'ControlPanel',
    components: { Panel, Control },
    props: {
      id: String,
      ns: Namespace,
      dataMap: Map
    },

    data() {
      return {
        data: null,
        retriever: null,
        node: null
      }
    },
    computed: {
      header() {
        return 'Controls: ' + (this.node ? this.node.getPretty() : '');
      },
      controllableNodes() {
        if (!this.node) return [];
        var c = [];
        for (let n of this.node.nodes()) {
          if (n instanceof Action || n instanceof Variable) c.push(n);
        }
        return c;
      }
    },
    methods: {
      async contextChanged(ctx) {
        if (!ctx) {
          this.node = null;
          return;
        }
        var root = await ctx.fetch();
        this.node = root.fromPath(this.data.node);
      }
    },
    created() {
      this.data = this.dataMap.get(this.id);
      this.retriever = new ContextRetriever({by_name:this.data.ctx}, 
                                              this.contextChanged);
      this.retriever.namespaceChanged(this.ns);
    },
    watch: { 
      ns(val) { this.retriever.namespaceChanged(val); } 
    },
    destroyed() {
      // cancel any feeds we have open
      this.retriever.stop();
    }
  }
</script>

<style>
  .controls {
    display: flex;
    flex-wrap: wrap;
  }
</style>
