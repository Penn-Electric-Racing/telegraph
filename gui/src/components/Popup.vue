<template>
	<div
		class="popup"
		:style="{
			left: x ? x : '50%',
			top: y ? y : '50%',
			width: width ? width : 'auto',
			height: height ? height : 'auto',
			maxWdith: '100%',
			maxHeight: '100%',
			transform: (x ? '' : 'translateX(-50%)') + (y ? '' : ' translateY(-50%)'),
		}"
		@click.stop="$emit('focus')"
	>
		<div class="popup-header">
			<span class="popup-name">{{ name }}</span>
			<span class="popup-close" @click.stop="$emit('close')">&times;</span>
		</div>
		<div class="popup-content">
			<slot></slot>
		</div>
	</div>
</template>

<script>
export default {
	name: "Popup",
	props: {
		x: Number,
		y: Number,
		width: Number,
		height: Number,
		name: String,
	},
};
</script>

<style scoped>
.popup {
	position: absolute;
	color: var(--main-text-color);
	display: flex;
	flex-direction: column;
	background-color: var(--sidebar-background-color);
	border-radius: 6px;
	box-shadow: 0 3px 9px var(--secondary-box-shadow);
}
.popup-header {
	padding: 10px 5px 10px 5px;
	border-radius: 6px 6px 0px 0px;
	background-color: #272c30;
	align-self: stretch;
	display: flex;
	flex-direction: row;
	align-items: center;
	justify-content: space-between;
	user-select: none;
}
.popup-close {
	font-size: 1.6rem;
	max-height: 1rem;
	transform: translateY(-0.4rem);
	margin: 0 0.2rem 0 0.7rem;
	transition: color 0.3s ease-in-out;
}
.popup-close:hover {
	color: #ed4949;
}
.popup-name {
	margin-left: 0.3rem;
}
.popup-content {
	margin: 5px 5px 5px 5px;
	min-height: 0px;
	display: flex;
}
</style>
