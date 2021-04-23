<template>
	<div class="action-input">
		<div class="number-input" v-if="this.node._argType.isNumber()">
			<input v-model.number="input" type="number" class="number-box" />
			<Button class="action-button" text="submit" @click="sendInput" />
			<span v-if="has_value" class="action-response">{{
				string_response
			}}</span>
		</div>
		<div class="number-input" v-if="this.node._argType.isBoolean()">
			<Button class="action-button" text="true" @click="sendVal(true)"></Button>
			<Button
				class="action-button"
				text="false"
				@click="sendVal(false)"
			></Button>
			<span v-if="has_value" class="action-response">{{
				string_response
			}}</span>
		</div>
		<div class="number-input" v-if="this.node._argType.isNone()">
			<Button
				class="action-button"
				text="send"
				@click="sendVal(undefined)"
			></Button>
			<span v-if="has_value" class="action-response">{{
				string_response
			}}</span>
		</div>
		<div
			class="number-input"
			v-if="
				this.node._argType.isEnum() && this.node._argType._labels.length > 2
			"
		>
			<Dropdown :changeHandler="onDropdownChange">
				<template v-slot:options>
					<option v-for="option in node._argType._labels" :key="option"
						>({{ node._argType._labels.indexOf(option) }}) {{ option }}</option
					>
				</template>
			</Dropdown>
			<Button class="action-button" text="submit" @click="sendInput()" />
			<span v-if="has_value" class="action-response">{{
				string_response
			}}</span>
		</div>
		<div
			class="number-input"
			v-if="
				this.node._argType.isEnum() && this.node._argType._labels.length <= 2
			"
		>
			<Button
				class="action-button"
				v-for="option in node._argType._labels"
				:key="option"
				:text="'(' + node._argType._labels.indexOf(option) + ') ' + option"
				@click="onOptionClick(option)"
			/>
			<span v-if="has_value" class="action-response">{{
				string_response
			}}</span>
		</div>
	</div>
</template>

<script>
	import { Node, Type } from "telegraph";
	import Button from "../components/Button.vue";
	import Dropdown from "../components/Dropdown.vue";
	export default {
		name: "ActionControl",
		components: { Button, Dropdown },
		props: {
			node: Node,
		},
		data() {
			return {
				input: 0,
				response: undefined,
				has_value: false,
				string_response: undefined,
			};
		},
		methods: {
			sendInput() {
				let res_prom = this.node.call(this.input);
				res_prom.then((res) => {
					this.has_value = true;
					this.response = res.v;
					this.formatResponse();
				});
			},
			sendVal(val) {
				let res_prom = this.node.call(val);
				res_prom.then((res) => {
					this.has_value = true;
					this.response = res.v;
					this.formatResponse();
				});
			},
			onDropdownChange(event) {
				let index = "";
				let i = 1;
				while (event.target.value[i] != ")") {
					index += event.target.value[i];
					i++;
				}
				this.input = Number(index);
			},
			onOptionClick(option) {
				this.sendVal(this.node._argType._labels.indexOf(option));
			},
			formatResponse() {
				console.log(this.node);
				if (this.node._retType.isEnum()) {
					this.string_response = this.node._retType._labels[this.response];
				} else {
					this.string_response = this.response;
				}
			},
		},
		created() {},
	};
</script>

<style>
	.action-button {
		margin: 10px;
	}
	.number-box {
		margin: 10px;
	}
	.actoin-response {
		margin: 10px;
	}
</style>
