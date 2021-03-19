<template>
	<Bubble hasContent ref="SettingsView">
		<template v-slot:header>
			<div class="items header-items">
				<span> {{ title }} </span>
			</div>
		</template>
		<template v-slot:content>
			<div
				class="items content-items"
				v-for="entry in entries"
				:key="entry.title"
			>
				<span class="content-type">
					{{ entry.title }}
				</span>
				<!-- if it is a toggle  -->
				<span v-if="entry.type == 'toggle'">
					<Toggle />
				</span>

				<span v-if="entry.type == 'info'">
					{{ entry.infoMessage }}
				</span>
			</div>
		</template>
	</Bubble>
</template>

<script>
import { Context } from "telegraph";

import Bubble from "../components/Bubble.vue";

// import toggle switches
import Toggle from "../components/Toggle.vue";

export default {
	name: "SettingsView",
	components: { Bubble, Toggle },
	props: {
		title: String,
		entries: Array,
	},
	methods: {},
};
</script>

<style scoped>
.items {
	display: flex;
	flex-direction: row;
	padding-right: 12px;
	flex: 1;
}
.content-items {
	justify-content: flex-start;
	padding-right: 10px;
	padding-left: 10px;
	padding-top: 3px;
	padding-bottom: 4px;
}
.header-items {
	justify-content: space-between;
}
.content-type {
	/* color: #1c8ed7; */
	color: var(--accent-color);
	width: 100%;
}
</style>
