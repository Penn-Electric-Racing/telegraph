<template>
	<ScrollArea> <SettingsView /> </ScrollArea>
</template>

<script>
import ScrollArea from "../components/ScrollArea.vue";
import SettingsView from "../views/SettingsView.vue";

export default {
	name: "SettingsPage",
	components: { ScrollArea, SettingsView },
	props: {},
};
</script>
