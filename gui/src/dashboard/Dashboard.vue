<template>
	<div class="dashboard" ref="dashboard">
		<ScrollArea>
			<div class="grid-container">
				<grid-layout
					:layout.sync="data.layout"
					:col-num="numCols"
					:row-height="rowHeight"
				>
					<grid-item
						v-for="item in data.layout"
						class="tile"
						:x="item.x"
						:y="item.y"
						:w="item.w"
						:h="item.h"
						:i="item.i"
						dragIgnoreFrom=".noDrag"
						:key="item.i"
					>
						<component
							v-bind:is="
								data.widgets[item.i] ? data.widgets[item.i].type : null
							"
							:id="item.i"
							:nsQuery="nsQuery"
							:data="data.widgets[item.i]"
							@close="remove(item.i)"
						/>
					</grid-item>
				</grid-layout>
			</div>
		</ScrollArea>
	</div>
</template>

<script>
import Vue from "vue";
import VueGridLayout from "vue-grid-layout";
import interact from "interactjs";
import { NamespaceQuery } from "telegraph";

import ScrollArea from "../components/ScrollArea.vue";

import uuidv4 from "uuid";

// the valid types
import Graph from "./Graph.vue";
import GraphMinMax from "./GraphMinMax.vue";
import ControlPanel from "./ControlPanel.vue";
import Placeholder from "./Placeholder.vue";

export default {
	name: "Dashboard",
	components: {
		ControlPanel,
		Graph,
		GraphMinMax,
		Placeholder,
		GridLayout: VueGridLayout.GridLayout,
		GridItem: VueGridLayout.GridItem,
		ScrollArea,
	},
	props: {
		name: String,
		id: String,
		nsQuery: NamespaceQuery,
		data: Object,
	},
	data: function() {
		return {
			numCols: 30,
			rowHeight: 30,
			dragOver: false,
		};
	},
	methods: {
		remove(id) {
			if (this.data.widgets) delete this.data.widgets[id];
			if (this.data.layout)
				this.data.layout.splice(
					this.data.layout.findIndex((x) => x.i == id),
					1
				);
		},
		drop(x, y, data) {
			var r = this.$refs["dashboard"].getBoundingClientRect();

			var col = Math.max(0, Math.round(x / (r.width / this.numCols)));
			var row = Math.max(0, Math.round(y / this.rowHeight));

			var tile = Vue.observable({
				type: "Placeholder",
				ctx: data.getContext().name,
				node: data.path(),
			});

			var tileUUID = uuidv4();
			if (!this.data.layout) this.data.layout = Vue.observable([]);
			if (!this.data.widgets) this.data.widgets = Vue.observable({});

			this.data.widgets[tileUUID] = tile;
			this.data.layout.push({ i: tileUUID, x: col, y: row, w: 4, h: 4 });
		},
	},
	mounted() {
		// listen for drop events
		interact(this.$refs["dashboard"])
			.dropzone({
				overlap: "pointer",
				accept: ".node-bubble",
			})
			.on("dragenter", (event) => {
				this.dragOver = true;
			})
			.on("dragleave", (event) => {
				this.dragOver = false;
			})
			.on("drop", (event) => {
				// if (event.target.type)
				console.log(event.target);
				var x = parseFloat(event.relatedTarget.getAttribute("data-x"));
				var y = parseFloat(event.relatedTarget.getAttribute("data-y"));

				var r = this.$refs["dashboard"].getBoundingClientRect();
				this.drop(x - r.left, y - r.top, event.interaction.data);
			});
	},
	created() {},
};
</script>

<style></style>

<style scoped>
.dashboard >>> .vue-grid-placeholder {
	background-color: var(--hover-color);
	opacity: 0.2;
	border: 2px solid var(--hover-color);
	border-radius: 3px;
	opacity: 1;
}
.dashboard {
	width: 100%;
	height: 100%;
	display: flex;
	flex-direction: column;
}
.drop-activated {
	opacity: 0.5;
}
.tile {
	background-color: var(--sidebar-background-color);
	color: var(--main-text-color);
	border: 0px;
	border-radius: 4px;
}
</style>
