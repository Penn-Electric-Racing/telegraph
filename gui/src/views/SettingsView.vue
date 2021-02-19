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
				<toggle-button
					v-if="entry.type == 'toggle'"
					:value="false"
					:color="{ checked: '#1c8ed7', unchecked: '#5e6870' }"
					:width="50"
					:height="20"
				/>
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
import { ToggleButton } from "vue-js-toggle-button";

import uuidv4 from "uuid/v4";
import Vue from "vue";

Vue.component("ToggleButton", ToggleButton);

export default {
	name: "SettingsView",
	components: { Bubble },
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
	color: #1c8ed7;
	width: 100%;
}
</style>
