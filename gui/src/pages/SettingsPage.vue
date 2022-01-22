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
						Change theme
					</span>
          <Dropdown :changeHandler="switchTheme">
            <template v-slot:options>
              <option value="dark">dark</option>
              <option value="light">light</option>
              <option value="red">red</option>
            </template>
          </Dropdown>
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
					<Dropdown :changeHandler="onDropdownChange">
						<template v-slot:options>
							<option>currently selected</option>
							<option>first option</option>
							<option>second option</option>
						</template>
					</Dropdown>
				</div>
				<div class="items content-items">
					<span class="content-type">
						New label
					</span>
					<Button v-tooltip="''" text="click me">
          </Button>
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
import Button from "../components/Button.vue";

export default {
	name: "SettingsPage",
	components: { ScrollArea, Bubble, Toggle, Dropdown, Button },
	props: {},
	methods: {
		switchTheme(event) {
			this.$store.dispatch("theme/editTheme", event.target.value);
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
