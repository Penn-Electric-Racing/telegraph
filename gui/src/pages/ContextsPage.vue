<template>
	<div class="contexts-page">
		<ScrollArea>
			<div class="contexts">
				<ContextView
					v-for="context in sortedContexts"
					:key="context.uuid"
					:context="context"
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
import ContextView from "../views/ContextView.vue";
import Bubble from "../components/Bubble.vue";
import ScrollArea from "../components/ScrollArea.vue";

import CreatePage from "./CreatePage.vue";

import { NamespaceQuery } from "telegraph";

import uuidv4 from "uuid/v4";
import Vue from "vue";

Vue.component("CreatePage", CreatePage);

export default {
	name: "ContextsPage",
	components: { ContextView, Bubble, ScrollArea },
	props: {
		nsQuery: NamespaceQuery,
	},
	data: function() {
		return {
			contextCollection: null,
			contexts: [],
		};
	},
	computed: {
		sortedContexts() {
			return this.contexts.slice().sort((a, b) => a.name.localeCompare(b.name));
		},
	},
	created() {
		this.nsUpdated();
	},
	methods: {
		nsUpdated() {
			if (this.nsQuery) {
				this.contextCollection = this.nsQuery.contexts.with_trees().collect();
				this.contexts = [];
				for (let c of this.contextCollection) this.contexts.push(c);
				this.contextCollection.added.add((x) => this.contexts.push(x));
				this.contextCollection.removed.add((x) =>
					this.contexts.splice(this.contexts.indexOf(x), 1)
				);
			} else {
				this.contexts = [];
				this.contextCollection = null;
			}
		},
		createNew() {
			// popup a creation dialogue
			this.$bubble("popup", {
				name: "Create Context",
				type: "CreatePage",
				id: uuidv4(),
				props: { resourceType: "Context" },
			});
		},
	},
	watch: {
		nsQuery() {
			this.nsUpdated();
		},
	},
};
</script>

<style scoped>
.contexts-page {
	height: 100%;
	width: 100%;
	display: flex;
	flex-direction: column;
	align-items: stretch;
}
.contexts {
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
