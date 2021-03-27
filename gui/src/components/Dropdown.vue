<template>
	<select class="currently-selected" @change="changeHandler">
		<slot name="options" />
	</select>
</template>

<script>
export default {
	name: "Dropdown",
	components: {},
	props: {
		changeHandler: {
			type: Function,
		},
		width: { type: Number, defalut: 350 },
	},
	methods: {},
};
</script>

<style>
.currently-selected {
	width: 350px;
	height: 30px;
	background-color: var(--dropdown-background);
	display: flex;
	align-items: center;
	justify-content: center;
	font-size: 15px;
	color: var(--contrast-color);
	border-radius: 5px;
}
</style>
