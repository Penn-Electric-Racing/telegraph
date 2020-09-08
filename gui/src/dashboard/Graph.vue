<template>
  <Panel @close="$emit('close')" :header="header" :noMargin="true">
    <div ref="chart" class="chart">
    </div>
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import { NamespaceQuery, Variable } from 'telegraph'
  import Plotly from 'plotly.js-dist'

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
        Plotly.newPlot(this.$refs['chart'],
          [{x: [], y: [], mode: 'lines', line: {color: '#1c8ed7'}}],
          {
            margin:{l:35, r:20, b: 35, t:20}, 
            plot_bgcolor:"#00000000",
            paper_bgcolor:"#00000000",
            xaxis: {
              color: '#ccc',
            },
            yaxis: {
              color: '#ccc',
            },
            modebar: {
              bgcolor: "#11111133",
            }
           },
          {responsive:true})
      },
      relayout() {
        Plotly.relayout(this.$refs['chart'], {width: this.width, height: this.height})
      },
      updateVariable(v) {
        if (!v || !(v instanceof Variable)) this.variable = [];
        else this.variables = [v];
        (async () => {
          if (this.sub) await this.sub.cancel();
          if (v) {
            this.sub = null;
            this.sub = await v.subscribe(0.05, 1);
            if (this.sub) {
              this.sub.data.add(v => {
                const d = new Date();
                const year = d.getFullYear();
                const month = (d.getMonth() + 1);
                const day = d.getDate();
                const hours = d.getHours().toString().padStart(2, "0");
                const mins = d.getMinutes().toString().padStart(2, "0");
                const sec = d.getSeconds().toString().padStart(2, "0");
                const milli = d.getMilliseconds().toString().padStart(3, "0");

                const dstr = `${year}-${month}-${day} ${hours}:${mins}:${sec}.${milli}`
                            
                Plotly.extendTraces(this.$refs['chart'], {
                  x: [[dstr]],
                  y: [[v]]
                }, [0])
              });
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