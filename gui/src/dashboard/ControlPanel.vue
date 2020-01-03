<template>
  <Panel @close="$emit('delete')" :header="header">
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import { Namespace, ContextRetriever } from 'telegraph'
  export default {
    name: 'Control',
    components: { Panel },
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
      }
    },
    methods: {
      async contextChanged(ctx) {
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
    watch: { ns(val) { this.retriever.namespaceChanged(val); } },
    destroyed() {
      // cancel any feeds we have open
      this.retriever.stop();
    }
  }
</script>

