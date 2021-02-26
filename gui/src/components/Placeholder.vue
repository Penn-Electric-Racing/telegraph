<template>
	<div class="placeholder">
		<p>{{ text }}</p>
	</div>
</template>

<script>
export default {
	name: "Placeholder",
	props: {
		text: String,
	},
};
</script>

<style scoped>
.placeholder {
	border: 2px dashed var(--tab-color);
	border-radius: 16px;
	box-sizing: border-box;

	margin: 10px;
	width: calc(100% - 20px);
	height: calc(100% - 20px);

	display: flex;
	justify-content: center;
	align-items: center;

	color: var(--tab-color);
	font-size: 2rem;
}
.placeholder p {
	text-align: center;
	margin: 5px;
}
</style>
