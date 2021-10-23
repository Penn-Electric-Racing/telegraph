<template>
	<Panel @close="$emit('close')" :header="header" :noMargin="true">
		<template v-slot:header>
			<div class="graph-controls">
				<NumberField v-model="timespan" @input="updateTimespan" />
				<FlatButton
					icon="clock"
					:class="{ controlActive: useTimespan }"
					@click="useTimespan = !useTimespan"
					title="Use Timespan"
				/>
				<FlatButton
					icon="play"
					:class="{ controlActive: live }"
					@click="live = !live"
					title="Play"
				/>
				<FlatButton
					icon="arrow-alt-circle-up"
					:class="{ controlActive: live }"
					@click="toggleMax"
					title="Reset Max Line"
				/>
				<FlatButton
					icon="arrow-alt-circle-down"
					:class="{ controlActive: live }"
					@click="toggleMin"
					title="Reset Min Line"
				/>
			</div>
		</template>
		<div ref="chart-container" class="chart-container">
			<div
				ref="chart"
				class="chart"
				:style="{ width: this.width + 'px', height: this.height + 'px' }"
			></div>
		</div>
	</Panel>
</template>

<script>
import Panel from "../components/Panel.vue";
import FlatButton from "../components/FlatButton.vue";
import NumberField from "../components/NumberField.vue";
import { NamespaceQuery, Variable } from "telegraph";
import TimeChart from 'timechart'


export default {
	name: "GraphMinMax",
	components: { Panel, FlatButton, NumberField },
	props: {
		id: String,
		nsQuery: NamespaceQuery,
		data: Object,
	},
	data() {
		return {
			variables: [],
			history: [],
			maxVals: [],
			maxVal: 0,
			maxVisible: true,
			minVals: [],
			minVal: 0,
			minVisible: true,
			chart: null,

			timespan: 20,
			useTimespan: true,
			live: true,

			width: 0,
			height: 0,
			sub: null,
		};
	},
	computed: {
		header() {
			return (
				"GraphMinMax: " + (this.variables[0] ? this.variables[0].getPretty() : "")
			);
		},
		nodeQuery() {
			return this.nsQuery.contexts
				.extract((x) => x.name == this.history.ctx)
				.fetch()
				.fromPath(this.data.node);
		},
	},
	mounted() {
		new ResizeObserver(() => {
			if (this.$refs["chart-container"]) {
				this.width = this.$refs["chart-container"].offsetWidth;
				this.height = this.$refs["chart-container"].offsetHeight;
				this.$nextTick(this.relayout());
			}
		}).observe(this.$refs["chart-container"]);
		this.width = this.$refs["chart-container"].offsetWidth;
		this.height = this.$refs["chart-container"].offsetHeight;
		this.setup();
	},
	unmounted() {},
	methods: {
		setup() {
			this.chart = new TimeChart(this.$refs["chart"], {
				series: [
					{ name : 'Series 1', data: this.history, color: 'blue' },
					{ name : 'Max', data: this.maxVals, color: 'red', visible: this.maxVisible},
					{ name : 'Min', data: this.minVals, color: 'green', visible: this.minVisible },
				],
				realTime: true,
				baseTime: Date.now() - performance.now(),
				xRange: { min: 0, max: 20 * 1000 },
				legend: false,
			});
		},
		relayout() {
			this.chart.model.resize(this.width, this.height);
		},
		toggleLive() {
		},
		updateTimespan() {
		},
		updateScale() {
		},
		updateVariable(v) {
		},
		toggleMax(){
			// this.maxVisible = !this.maxVisible; 
			// for (const s of this.chart.options.series) {
			// 	if (s.name == 'Max') {
			// 		s.visible = !s.visible;
			// 	}
			// }
			this.maxVal = 0;
			// this.maxVals = [];
			this.chart.update();
		},
		toggleMin(){
			// this.minVisible = !this.minVisible; 
			// for (const s of this.chart.options.series) {
			// 	if (s.name == 'Min') {
			// 		s.visible = !s.visible;
			// 	}
			// }
			this.minVal = 0;
			// this.minVals = [];
			this.chart.update();
		},
	},
	watch: {
		nodeQuery(n, o) {
			// if (o) o.unregister(this.updateVariable);
			// n.updated.register(this.updateVariable);
		},
	},
	created() {
		// callback for adding data
		setInterval(() => {
			const time = performance.now();
			var yVal = Math.sin(time * 0.002);
			this.history.push({x: time, y: yVal});
			if (this.maxVisible && this.maxVal < yVal) {
				this.maxVal = yVal;
			}
			if (this.minVisible && this.minVal > yVal) {
				this.minVal = yVal;
			}
			this.maxVals.push({x: time, y: this.maxVal});
			this.minVals.push({x: time, y: this.minVal});

			this.chart.update();
		}, 100);
		this.nodeQuery.register(this.updateVariable);
	},
	destroyed() {
		if (this.sub) this.sub.cancel();
	},
};
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
	color: var(--main-text-color);
}

.controlActive {
	color: var(--contrast-color);
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
	font-family: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue",
		Arial, "Noto Sans", sans-serif, "Apple Color Emoji", "Segoe UI Emoji",
		"Segoe UI Symbol", "Noto Color Emoji";
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
	background: var(--contrast-color);
	opacity: 0.1;
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
	border-right: 1px dashed var(--graph-border-color);
}

.u-cursor-y {
	width: 100%;
	border-bottom: 1px dashed var(--graph-border-color);
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
