<template>
  <Panel @close="$emit('close')" :header="header">
    <div class="controls">
      <Control :node="n" v-for="n in controllableNodes" :key="n.path().join('.')"/>
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
      data: Object
    },

    data() {
      return {
        node: null,
      }
    },
    created() {
      console.log('created!', this.data);
    },
    methods: {
      updateNode(node) {
        this.node = node
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
        return this.nsQuery.contexts
              .extract(x => x.name == this.data.ctx)
              .fetch()
              .fromPath(this.data.node);
      }
    },
    watch: {
      nodeQuery(n, o) {
        if (o) {
          o.updated.remove(this.updateNode);
        }
        this.updateNode(n.current);
        n.updated.add(this.updateNode);
      }
    },
    created() {
      this.updateNode(this.nodeQuery.current);
      this.nodeQuery.updated.add(this.updateNode);
    }
  }
</script>

<style>
  .controls {
    display: flex;
    flex-wrap: wrap;
  }
</style>
