<template>
	<div class="panel" v-else>
		<div class="panel-header">
			<div class="panel-header-text">{{ header }}<br /></div>
			<slot name="header"> </slot>
			<div class="panel-close noDrag" @click="$emit('close')">
				<div class="close-1" />
				<div class="close-2" />
			</div>
		</div>
		<div class="panel-content noDrag" :class="{ noMargin: noMargin }">
			<slot></slot>
		</div>
	</div>
</template>
<script>
export default {
	name: "Panel",
	props: {
		header: { type: String, default: "" },
		noMargin: { type: Boolean, default: false },
	},
};
</script>

<style scoped>
.panel {
	background-color: var(--sidebar-background-color);
	color: var(--main-text-color);
	height: 100%;

	border: 0px;
	border-radius: 4px;
	box-shadow: 2px 2px 5px 2px var(--secondary-background-color);
	display: flex;
	flex-direction: column;
	align-content: stretch;
}

.panel-header {
	border-bottom: 2px solid var(--accent-color);
	background-color: var(--panel-header-background);
	opacity: 0.5;
	color: var(--contrast-color);

	display: flex;
	align-items: center;
	overflow: hidden;
}
.panel-header-text {
	font-size: 12px;
	line-height: 16px;
	font-weight: normal;
	text-transform: uppercase;
	white-space: nowrap;
	overflow: hidden;
	text-overflow: ellipsis;
	padding-top: 11px;
	padding-bottom: 9px;
	padding-left: 20px;
	width: 100%;
}
.panel-close {
	position: relative;
	cursor: pointer;
	max-width: 20px;
	min-width: 20px;
	height: 20px;
	margin-right: 11px;
	color: var(--accent-color);
}

.panel-close:hover > * {
	background-color: #ed4949;
}
.close-1,
.close-2 {
	position: absolute;
	left: 15px;
	height: 20px;
	width: 2px;
	background-color: var(--tab-color);
	transition: background-color 0.5s;
}
.close-1 {
	transform: rotate(-45deg);
}
.close-2 {
	transform: rotate(45deg);
}

.panel-content {
	padding-top: 11px;
	padding-right: 20px;
	padding-bottom: 9px;
	padding-left: 20px;
	flex: 1;
	min-height: 0;
	box-sizing: border-box;
	overflow: hidden;
}
.noMargin {
	padding: 0;
}
</style>
