<template>
	<div class="currently-selected">currently selected</div>
</template>

<script>
export default {
	name: "Dropdown",
	components: {},
	props: {},
	methods: {},
	data: {},
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
	border-radius: 30px;
}
</style>
