<template>
	<toggle-button
		:value="false"
		:color="{ checked: checked_color, unchecked: unchecked_color }"
		:width="width"
		:height="height"
		@change="onSwitch"
	/>
</template>

<script>
import { ToggleButton } from "vue-js-toggle-button";

import uuidv4 from "uuid/v4";
import Vue from "vue";

Vue.component("ToggleButton", ToggleButton);

export default {
	name: "Toggle",
	props: {
		checked_color: { type: String, default: "#1c8ed7" },
		unchecked_color: { type: String, default: "#5e6870" },
		width: { type: Number, defalt: 50 },
		height: { type: Number, defalt: 20 },
		checked: { type: Boolean, defalt: false },
		onSwitch: Function,
	},
};
</script>
