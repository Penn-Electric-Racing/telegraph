<template>
  <Panel @close="$emit('close')" :header="header">
    <div ref="chart" class="chart">
    </div>
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import { NamespaceQuery, Variable } from 'telegraph'
  import * as Plotly from 'plotly'

  export default {
    name: 'Graph',
    components: { Panel  },
    props: {
      id: String,
      nsQuery: NamespaceQuery,
      data: Object
    },
    data() {
      return {
        variables: [],
        sub: null,
        history: []
      }
    },
    computed: {
      header() {

      },
      nodeQuery() {
        return this.nsQuery.contexts
            .extract(x => x.name == this.data.ctx)
            .fetch()
            .fromPath(this.data.node)
      }
    },
    mounted() {
      new ResizeObserver(() => {
        if (this.$refs['chart']) {
          this.width = this.$refs['chart'].offsetWidth;
          this.height = this.$refs['chart'].offsetHeight;
          this.relayout();
        }
      }).observe(this.$refs['chart']);
      this.setup();
    },
    unmounted() {
      Plotly.purge(this.$refs['chart']);
    },
    methods: {
      setup() {
        Plotly.newPlot(this.$refs['chart'])
      },
      relayout() {

      },
      updateVariable(v) {
        if (!v || !(v instanceof Variable)) this.variable = [];
        else this.variables = [v];
        (async () => {
          if (this.sub) await this.sub.cancel();
          if (v) {
            this.sub = null;
            this.sub = await v.subscribe(0.5, 1);
            if (this.sub) {
              this.sub.data.add(v => this.history.push(v));
              this.sub.poll();
            }
          }
        })();
      }
    },
    watch: {
      nodeQuery(n, o) {
        if (o) o.unregister(this.updateVariable);
        n.updated.register(this.updateVariable);
      }
    },
    created() {
      this.nodeQuery.register(this.updateVariable);
    },
    destroyed() {
      if (this.sub) this.sub.cancel();
    }
  }
</script>

<style scoped>
.chart {
  width: 100%;
  height: 100%;
}
</style>