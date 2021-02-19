<template>
	<Bubble hasContent>
		<template v-slot:header>
			<div class="items header-items">
				<span> {{ "Appearance" }} </span>
			</div>
		</template>
		<template v-slot:content>
			<div class="items content-items">
				<span class="content-type">
					{{ "Theme (dark/light)" }}
				</span>
				<toggle-button
					:value="false"
					:color="{ checked: '#1c8ed7', unchecked: '#5e6870' }"
					:width="50"
					:height="20"
				/>
			</div>
			<div class="items content-items">
				<span class="content-type">
					{{ "Version" }}
				</span>
				0.0.1
			</div>
		</template>
	</Bubble>
</template>

<script>
import { Context } from "telegraph";

import Bubble from "../components/Bubble.vue";

// globally register the componentinfopage
import ComponentInfoPage from "../pages/ComponentInfoPage.vue";
import StreamRequestPage from "../pages/StreamRequestPage.vue";

// import toggle switches
import { ToggleButton } from "vue-js-toggle-button";

import uuidv4 from "uuid/v4";
import Vue from "vue";

Vue.component("ComponentInfoPage", ComponentInfoPage);
Vue.component("StreamRequestPage", StreamRequestPage);
Vue.component("ToggleButton", ToggleButton);

export default {
	name: "ComponentView",
	components: { Bubble },
	props: {
		// component: Context,
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
