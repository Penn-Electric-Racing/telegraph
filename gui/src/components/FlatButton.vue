<template>
	<button class="flat-button" v-on="$listeners">
		<font-awesome-icon :icon="icon" :size="iconSize" v-if="icon != undefined" />
	</button>
</template>

<script>
export default {
	name: "FlatButton",
	props: {
		icon: { type: String, default: undefined },
		iconSize: { type: String, default: undefined },
	},
};
</script>

<style scoped>
.flat-button:hover {
	background-color: var(--flat-button-hover-color);
}
.flat-button:focus {
	outline: 0;
}
.flat-button {
	display: inline-block;
	border: 0;
	border-radius: 5px;
	background-color: transparent;
	padding: 5px;
	color: inherit;

	pointer-events: all;
	cursor: pointer;

	transition: background-color 0.5s ease-in-out, color 0.5s ease-in-out;
}
</style>
