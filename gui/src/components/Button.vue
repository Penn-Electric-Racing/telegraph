<template>
	<button class="button" v-on="$listeners">
		{{ text }}
	</button>
</template>

<script>
export default {
	name: "Button",
	props: {
		text: String,
	},
};
</script>

<style scoped>
.button {
	transition: background-color 0.15s ease-in-out;
	display: inline-block;
	border: none;
	color: var(--contrast-color);
	background-color: var(--button-background-color);
	font-size: 0.9rem;
	padding: 4px 8px 4px 8px;
	border-radius: 5px;
	min-height: calc(1rem + 12px);
}
.button:hover {
	background-color: var(--button-hover-color);
}
.button:focus {
	outline: 0;
}
.button:active {
	background-color: var(--button-active-color);
}
</style>
