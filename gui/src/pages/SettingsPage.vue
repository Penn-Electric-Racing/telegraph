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

						// document.documentElement.setAttribute("theme", "dark");
						// console.log(document.documentElement.getAttribute("theme"));
						if (document.documentElement.getAttribute("theme") == "light") {
							document.documentElement.setAttribute("theme", null);
						} else {
							document.documentElement.setAttribute("theme", "light");
						}
					},
				},
				{ title: "Version", type: "info", infoMessage: "0.0.1" },
			],
		};
	},
	methods: {},
};
</script>
