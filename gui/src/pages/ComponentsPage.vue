<template>
	<div class="components-page">
		<ScrollArea>
			<div class="components">
				<ComponentView
					v-for="component in sortedComponents"
					:key="component.uuid"
					:component="component"
				/>
			</div>
		</ScrollArea>
		<Bubble @click="createNew" class="plus-bubble">
			<template v-slot:header>
				<div class="plus">
					<font-awesome-icon icon="plus" />
				</div>
			</template>
		</Bubble>
	</div>
</template>

<script>
import ComponentView from "../views/ComponentView.vue";
import Bubble from "../components/Bubble.vue";
import ScrollArea from "../components/ScrollArea.vue";

import CreatePage from "./CreatePage.vue";

import { NamespaceQuery } from "telegraph";

import uuidv4 from "uuid/v4";
import Vue from "vue";

Vue.component("CreatePage", CreatePage);

export default {
	name: "ComponentsPage",
	components: { ComponentView, Bubble, ScrollArea },
	props: {
		nsQuery: NamespaceQuery,
	},
	data: function() {
		return {
			componentCollection: null,
			components: [],
		};
	},
	computed: {
		sortedComponents() {
			return this.components
				.slice()
				.sort((a, b) => a.name.localeCompare(b.name));
		},
	},
	created() {
		if (this.nsQuery) {
			this.componentCollection = this.nsQuery.contexts.headless().collect();
			this.componentCollection.added.add((x) => this.components.push(x));
			this.componentCollection.removed.add((x) =>
				this.components.splice(this.components.indexOf(x), 1)
			);
		} else {
			this.componentCollection = null;
		}
	},
	methods: {
		createNew() {
			// popup a creation dialogue
			this.$bubble("popup", {
				name: "Create Component",
				type: "CreatePage",
				id: uuidv4(),
				props: { resourceType: "Component" },
			});
		},
	},
	watch: {
		nsQuery() {
			if (this.nsQuery) {
				this.componentCollection = this.nsQuery.components.collect();
				this.componentCollection.added.add((x) => this.components.push(x));
				this.componentCollection.removed.add((x) =>
					this.components.splice(this.components.indexOf(x), 1)
				);
			} else {
				this.componentCollection = null;
			}
		},
	},
};
</script>

<style scoped>
.components-page {
	height: 100%;
	width: 100%;
	display: flex;
	flex-direction: column;
	align-items: stretch;
}
.components {
	width: 100%;
	display: flex;
	flex-direction: column;
	align-items: stretch;
}
.plus {
	font-size: 1.2rem;
	text-align: center;
	flex: 1;
}
.plus-bubble {
	transition: background-color 0.5s ease-in-out, color 0.5s ease-in-out;
}
.plus-bubble:hover {
	color: var(--accent-color);
}
</style>
