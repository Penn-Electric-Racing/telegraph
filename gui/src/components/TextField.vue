<template>
	<input
		class="text-field"
		:placeholder="placeholder"
		:value="inputValue"
		@input="updateValue($event.target.value)"
		@keyup.enter="submit"
	/>
</template>

<script>
export default {
	name: "TextField",
	props: {
		placeholder: { type: String, default: "Type Text..." },
		inputValue: String,
	},
	methods: {
		updateValue(val) {
			this.$emit("input", val);
		},
		submit() {
			this.$emit("submit");
		},
	},
	mounted() {
		// init setup with default values
		this.$emit("input", this.inputValue);
	},
};
</script>

<style scoped>
.text-field {
	padding: 4px 3px 2px 4px;
	border: none;
	color: var(--textfield-color);
	background-color: var(--textfield-border);
	border: 1px solid var(--secondary-background-color);
	box-shadow: inset 1px 1px 2px 2px var(--secondary-background-color);
	font-size: 1rem;
}
.text-field:focus {
	color: var(--contrast-color);
	outline: 0;
}
</style>
