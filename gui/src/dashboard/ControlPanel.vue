<template>
  <Panel @close="$emit('delete')" :header="header">
    <div class="controls">
      <Control :node="n" v-for="n in controllableNodes" :key="n.getName()"/>
    </div>
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import Control from './Control.vue'
  import { NamespaceQuery, Action, Variable, ContextRetriever } from 'telegraph'
  export default {
    name: 'ControlPanel',
    components: { Panel, Control },
    props: {
      id: String,
      nsQuery: NamespaceQuery,
      dataMap: Map
    },

    data() {
      return {
        node: null,
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
      },
      nodeQuery() {
        return null;
      }
    }
  }
</script>

<style>
  .controls {
    display: flex;
    flex-wrap: wrap;
  }
</style>
