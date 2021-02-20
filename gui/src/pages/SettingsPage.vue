<template>
	<ScrollArea>
		<SettingsView title="Appearance" :entries="appearanceEntries" />
	</ScrollArea>
</template>

<script>
import ScrollArea from "../components/ScrollArea.vue";
import SettingsView from "../views/SettingsView.vue";

export default {
	name: "SettingsPage",
	components: { ScrollArea, SettingsView },
	props: {},
	data() {
		return {
			appearanceEntries: [
				{
					title: "Theme (dark/light)",
					type: "toggle",
					onSwitch: () => {
						console.log("switching");
					},
				},
				{ title: "Version", type: "info", infoMessage: "0.0.1" },
			],
		};
	},
	methods: {},
};
</script>
