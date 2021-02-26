<template>
	<input
		class="text-field"
		:value="value"
		@input="updateValue($event.target.value)"
		@keyup.enter="submit"
		size="3"
	/>
</template>

<script>
export default {
	name: "NumberField",
	props: {
		value: { type: Number, default: 0 },
	},
	methods: {
		updateValue(val) {
			let value = Number.parseInt(val);
			if (isNaN(value)) return;
			this.$emit("input", value);
		},
		submit() {
			this.$emit("submit");
		},
	},
};
</script>

<style scoped>
.text-field {
	padding: 4px 3px 2px 4px;
	border: none;
	color: var(--textfield-color);
	background-color: var(--textfield-border);
	border: 1px solid var(--secondaryh-background-color);
	box-shadow: inset 1px 1px 2px 2px var(--main-box-shadow);
	font-size: 1rem;
}
.text-field:focus {
	color: var(--contrast-color);
	outline: 0;
}
</style>
