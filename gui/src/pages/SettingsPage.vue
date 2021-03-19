<template>
	<ScrollArea>
		<Bubble hasContent ref="SettingsView">
			<template v-slot:header>
				<div class="items header-items">
					<span> Appearance </span>
				</div>
			</template>
			<template v-slot:content>
				<div class="items content-items">
					<span class="content-type">
						Theme (dark/light)
					</span>
					<Toggle :onSwitch="switchTheme" />
				</div>
				<div class="items content-items">
					<span class="content-type">
						Version
					</span>
					<span class="info-type">
						0.0.1
					</span>
				</div>
			</template>
		</Bubble>
	</ScrollArea>
</template>

<script>
import ScrollArea from "../components/ScrollArea.vue";
import Bubble from "../components/Bubble.vue";
import Toggle from "../components/Toggle.vue";

export default {
	name: "SettingsPage",
	components: { ScrollArea, Bubble, Toggle },
	props: {},
	methods: {
		switchTheme() {
			// if (document.documentElement.getAttribute("theme") == "light") {
			// 	document.documentElement.setAttribute("theme", null);
			// } else {
			// 	document.documentElement.setAttribute("theme", "light");
			// }
			this.$store.dispatch("theme/swapTheme");
		},
	},
};
</script>

<style scoped>
.header-items {
	justify-content: space-between;
}
.content-items {
	justify-content: flex-start;
	padding-right: 10px;
	padding-left: 10px;
	padding-top: 3px;
	padding-bottom: 4px;
}
.content-type {
	/* color: #1c8ed7; */
	color: var(--accent-color);
	width: 100%;
}
.items {
	display: flex;
	flex-direction: row;
	padding-right: 12px;
	flex: 1;
}
</style>
