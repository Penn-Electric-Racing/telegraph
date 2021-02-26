<template>
	<div class="tabarea">
		<slot></slot>
	</div>
</template>

<style scoped>
.tabarea {
	flex: 1;
	display: flex;
	flex-direction: column;
}
.header {
	box-sizing: border-box;
	height: 50px;
	background-color: var(--secondary-background-color);

	display: flex;
	align-items: center;
	justify-content: center;

	padding-left: 10px;
	padding-right: 10px;
}
.header.active {
	border-bottom: 2px solid var(--accent-color);
	color: var(--contrast-color);
}
</style>

<script>
export default {
	name: "TabArea",
	methods: {},
};
</script>
