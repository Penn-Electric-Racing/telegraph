<template>
	<div class="action-input">
		<div class="number-input" v-if="this.node._argType.isNumber()">
			<input v-model.number="input" type="number" class="number-box">
			<Button text="submit"></Button>
		</div>
	</div>
</template>

<script>
import { Node, Type } from "telegraph";
import Button from "../components/Button.vue";
export default {
	name: "ActionControl",
	components: { Button },
	props: {
		node: Node
	},
	created() {
	},
};
</script>

<style>
</style>