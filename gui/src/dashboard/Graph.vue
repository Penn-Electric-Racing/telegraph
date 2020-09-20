<template>
  <Panel @close="$emit('close')" :header="header" :noMargin="true">
    <template v-slot:header>
      <div class="graph-controls">
        <NumberField v-model="timespan" @input="updateTimespan"/>
        <FlatButton icon="clock" :class="{controlActive: useTimespan}" 
            @click="useTimespan = !useTimespan"/>
        <FlatButton icon="play" :class="{controlActive: live}"
            @click="live = !live"/>
      </div>
    </template>
    <div ref="chart-container" class="chart-container">
      <div ref="chart" class="chart" :style="{'width': this.width+'px', 'height': this.height+'px'}">
      </div>
    </div>
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  import FlatButton from '../components/FlatButton.vue'
  import NumberField from '../components/NumberField.vue'
  import { NamespaceQuery, Variable } from 'telegraph'
  import uPlot from 'uplot'

  import { wheelZoomPlugin } from './graph-plugins.js'



  export default {
    name: 'Graph',
    components: { Panel, FlatButton, NumberField },
    props: {
      id: String,
      nsQuery: NamespaceQuery,
      data: Object
    },
    data() {
      return {
        variables: [],
        history: [[],[]],

        timespan: 0,
        useTimespan: false,
        live: true,

        width: 0,
        height: 0,
        sub: null,
      }
    },
    computed: {
      header() {
        return 'Graph: ' + (this.variables[0] ? this.variables[0].getPretty() : '');
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
        if (this.$refs['chart-container']) {
          this.width = this.$refs['chart-container'].offsetWidth;
          this.height = this.$refs['chart-container'].offsetHeight;
          this.relayout();
        }
      }).observe(this.$refs['chart-container']);
      this.width = this.$refs['chart-container'].offsetWidth;
      this.height = this.$refs['chart-container'].offsetHeight;
      this.setup();
    },
    unmounted() {
    },
    methods: {
      setup() {
        const opts = {
          width: this.width,
          height: this.height,
          scales: { x: { time: false, min: null, max: null, auto: false }, y: { auto: true }},
          axes: [
            {stroke: "#fff", grid: {stroke: "rgb(80, 80, 80)"}},
            {stroke: "#fff", grid: {stroke: "rgb(80, 80, 80)"}}
          ],
          series: [
            {},
            {label: "None", stroke: "#1c8ed7", width: 1}
          ],
          plugins: [
            wheelZoomPlugin({
              factor: 0.8,
              scrollCallback: (oldMin, oldMax, newMin, newMax) => {
                if (this.useTimespan && this.live) {
                  console.log(oldMin);
                  this.timespan = newMax - newMin;
                  return {min: oldMax - (newMax - newMin), max: oldMax};
                }
              },
              canScroll: () => {
                return !this.live || (this.live && this.useTimespan);
              },
              canMove: () => { return !this.live }
            })
          ]
        }
        this.plot = new uPlot(opts, this.history, this.$refs['chart'])
      },
      relayout() {
        this.plot.setSize({width: this.width, height: this.height})
      },
      toggleLive() {
        this.live = !this.live;
        if (this.live) {
          this.plot.batch(() => {
            this.updateScale();
          });
        }
      },
      updateTimespan() {
        if (this.live && this.useTimespan) {
          this.plot.batch(() => {
            this.updateScale();
          })
        }
      },
      updateScale() {
        let nxMin = this.history[0] ? this.history[0][0] : null;
        let nxMax = this.history[0] ? this.history[0][this.history[0].length - 1] : null;
        if (this.useTimespan && this.timespan > 0) {
          nxMin = nxMax - this.timespan;
        }
        this.plot.setScale("x", {
            min: nxMin,
            max: nxMax,
        });
      },
      updateVariable(v) {
        if (!v || !(v instanceof Variable)) this.variable = [];
        else this.variables = [v];
        (async () => {
          if (this.sub) await this.sub.cancel();
          if (v) {
            this.sub = null;
            this.sub = await v.subscribe(0.0001, 1);
            if (this.sub) {
              this.sub.data.add(dp => {
                // this.history[0].push(dp.t / 1000000)
                this.history[0].push(this.history[0].length)
                this.history[1].push(dp.v)
                if (this.live) {
                  this.updateScale();
                }
                this.plot.setData(this.history, this.live);
                if (!this.live) this.plot.redraw();
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
.chart-container {
  width: 100%;
  height: 100%;
}
.graph-controls > input {
  text-align: right;
}
.graph-controls {
  display: flex;
  flex-direction: row;
  color: #88939C;
}

.controlActive {
  color: #fff;
}

.chart {
  display: block;
}
</style>

<style>
uplot,
.uplot *,
.uplot *::before,
.uplot *::after {
	box-sizing: border-box;
}

.uplot {
	font-family: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji", "Segoe UI Symbol", "Noto Color Emoji";
	line-height: 1.5;
	width: max-content;
}

.u-title {
	text-align: center;
	font-size: 18px;
	font-weight: bold;
}

.u-wrap {
	position: relative;
	user-select: none;
}

.u-over,
.u-under {
	position: absolute;
	overflow: hidden;
}

.uplot canvas {
	display: block;
	position: relative;
	width: 100%;
	height: 100%;
}

.u-legend {
	font-size: 14px;
	margin: auto;
	text-align: center;
}

.u-inline {
	display: block;
}

.u-inline * {
	display: inline-block;
}

.u-inline tr {
	margin-right: 16px;
}

.u-legend th {
	font-weight: 600;
}

.u-legend th > * {
	vertical-align: middle;
	display: inline-block;
}

.u-legend .u-marker {
	width: 1em;
	height: 1em;
	margin-right: 4px;
	border: 2px solid transparent;
}

.u-inline.u-live th::after {
	content: ":";
	vertical-align: middle;
}

.u-inline:not(.u-live) .u-value {
	display: none;
}

.u-series > * {
	padding: 4px;
}

.u-series th {
	cursor: pointer;
}

.u-legend .u-off > * {
	opacity: 0.3;
}

.u-select {
	background: rgba(255,255,255,0.1);
	position: absolute;
	pointer-events: none;
}

.u-select.u-off {
	display: none;
}

.u-cursor-x,
.u-cursor-y {
	position: absolute;
	left: 0;
	top: 0;
	pointer-events: none;
	will-change: transform;
	z-index: 100;
}

.u-cursor-x {
	height: 100%;
	border-right: 1px dashed #98adb8;
}

.u-cursor-y {
	width: 100%;
	border-bottom: 1px dashed #98adb8;
}

.u-cursor-pt {
	position: absolute;
	top: 0;
	left: 0;
	border-radius: 50%;
	filter: brightness(85%);
	pointer-events: none;
	will-change: transform;
	z-index: 100;
}
</style>