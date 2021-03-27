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
				<div class="items content-items">
					<span class="content-type">
						Different Options
					</span>
					<!-- <Dropdown /> -->
					<Dropdown :changeHandler="onDropdownChange">
						<template v-slot:options>
							<option>currently selected</option>
							<option>first option</option>
							<option>second option</option>
						</template>
					</Dropdown>
				</div>
			</template>
		</Bubble>
	</ScrollArea>
</template>

<script>
import ScrollArea from "../components/ScrollArea.vue";
import Bubble from "../components/Bubble.vue";
import Toggle from "../components/Toggle.vue";
import Dropdown from "../components/Dropdown.vue";

export default {
	name: "SettingsPage",
	components: { ScrollArea, Bubble, Toggle, Dropdown },
	props: {},
	methods: {
		switchTheme() {
			this.$store.dispatch("theme/swapTheme");
		},
		onDropdownChange(event) {
			alert(event.target.value);
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
