<template>
	<Bubble
		v-if="isGroup"
		class="node-bubble"
		draggable
		hasContent
		:dragData="node"
		indent
	>
		<template v-slot:header>
			<span class="node-name">{{ node.getName() }}</span>
		</template>
		<template v-slot:content>
			<NodeView
				v-for="n in children"
				:node="n"
				:filter="filter"
				:showRecord="showRecord"
				:recording="recording"
				:key="n.getName()"
			/>
		</template>
	</Bubble>

	<Bubble
		v-else-if="isAction"
		class="node-bubble"
		:draggable="true"
		:dragData="node"
	>
		<template v-slot:header>
			<span class="node-name">{{ node.getName() }}</span>
		</template>
	</Bubble>
	<Bubble
		v-else-if="isVariable"
		class="node-bubble"
		:draggable="true"
		:dragData="node"
	>
		<template v-slot:header>
			<span class="node-name">{{ node.getName() }}</span>
			<div class="header-icons">
				<font-awesome-icon
					icon="circle"
					class="icon record-icon noBubbleDrag"
					@click="record"
					v-if="!isRecording && showRecord"
				/>
				<font-awesome-icon
					icon="square"
					class="icon stop-record-icon noBubbleDrag"
					@click="stopRecord"
					v-if="isRecording && showRecord"
				/>
			</div>
		</template>
	</Bubble>
</template>

<script>
import { Node, Group, Variable, Action } from "telegraph";
import Bubble from "../components/Bubble.vue";

export default {
	name: "NodeView",
	components: { Bubble },
	computed: {
		isGroup() {
			return this.node instanceof Group;
		},
		isAction() {
			return this.node instanceof Action;
		},
		isVariable() {
			return this.node instanceof Variable;
		},
		isRecording() {
			return this.node
				? this.recording.indexOf("/" + this.node.path().join("/")) >= 0
				: false;
		},
		children() {
			let children = this.node.getChildren ? this.node.getChildren() : [];
			let f = [];
			if (!this.filter) {
				f = children;
			} else {
				let anyOf = this.filter.split(" ");
				for (let c of children) {
					var allow = false;
					for (let d of c.nodes()) {
						var path = "/" + d.path().join(".");
						for (let p of anyOf) {
							var parts = p.split("/");
							let d = "";
							if (parts.length > 0 && parts[0].length == 0) {
								parts.shift();
								d = "/";
							}
							d = d + parts.join(".");
							if (path.indexOf(d) >= 0) {
								allow = true;
								break;
							}
						}
						if (allow) break;
					}
					if (allow) f.push(c);
				}
			}
			return f;
		},
	},
	props: {
		node: Node,
		filter: { type: String, default: null },
		recording: { type: Array, default: () => [] },
		showRecord: { type: Boolean, default: false },
	},
	methods: {
		record() {},
		stopRecord() {},
	},
};
</script>

<style scoped>
ul.tree-wiew > li {
	list-style-type: none;
	padding-bottom: 1rem;
	color: var(--contrast-color);
}

.header-icons {
	flex: 1;
	display: flex;
	justify-content: flex-end;
}

.node-name {
	user-select: none;
	text-overflow: ellipsis;
	overflow: hidden;
}

.icon {
	padding: 3px;
	padding-right: 0.4rem;
	font-size: 0.7rem;
	transition: color 0.3s ease-in-out;
}

.record-icon {
	color: var(--node-record-icon);
}
.record-icon:hover {
	color: #ed4949;
}
</style>
