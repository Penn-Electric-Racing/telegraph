<template>
	<ScrollArea>
		<div class="dashboard-list">
			<div v-for="(dash, id) in dashboards" class="dashboard-item" :key="id">
				{{ dash.info.name }}
				<div class="icons">
					<font-awesome-icon
						class="icon open"
						@click="open(id)"
						icon="folder-open"
					/>
					<font-awesome-icon
						class="icon destroy"
						@click="destroy(id)"
						icon="trash"
					/>
				</div>
			</div>
		</div>
	</ScrollArea>
</template>

<script>
import { NamespaceQuery } from "telegraph";
import ScrollArea from "../components/ScrollArea.vue";
import Vue from "vue";
export default {
	name: "DashboardsPage",
	components: { ScrollArea },
	props: {
		nsQuery: NamespaceQuery,
		dashboards: Object,
	},
	methods: {
		open(id) {
			this.$bubble("newtab", {
				type: "Dashboard",
				name: this.dashboards[id].info.name,
				id: id,
				props: { data: this.dashboards[id] },
			});
		},
		destroy(id) {
			Vue.delete(this.dashboards, id);
			// close the tab if it is open
			this.$bubble("closetab", id);
		},
	},
};
</script>

<style scoped>
.dashboard-list {
	padding: 5px;
	display: flex;
	flex-direction: column;
	align-items: stretch;
}
.dashboard-item {
	margin: 3px;
	padding: 3px;

	display: flex;
	flex-direction: row;
	align-items: center;
	justify-content: space-between;

	box-shadow: 0px 0px 10px var(--main-box-shadow);
	background-color: var(--secondary-background-color);
	border-radius: 3px;
	color: var(--tab-color);
}
.icon {
	margin: 3px;
	transition: color 0.3s ease-in-out;
	font-size: 0.8rem;
}
.icon.open:hover {
	color: var(--contrast-color);
}
.icon.destroy:hover {
	color: #ed4949;
}
</style>
