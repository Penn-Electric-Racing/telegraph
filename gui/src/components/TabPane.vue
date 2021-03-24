<template>
	<div class="tabpane">
        <TabGroup
            v-for="tg in tabGroups"
            :key="tg.index"
            :tabgroup="tg"
            :closeable="true"
            :editable="true"
            :draggable="true"
            :nsQuery="nsQuery"
            :dashboards="dashboards"
        />
	</div>
</template>

<style scoped>
    .tabpane {
        flex: 1;
        display: flex;
        flex-direction: column;
        min-height: 100%;
        min-width: 100%;
        height: 100%;
        width: 100%;
        position: relative;
    }
</style>

<script>
import TabGroup from "./TabGroup.vue";
import { NamespaceQuery } from 'telegraph';

export default {
	name: "TabPane",
    components: {TabGroup},
	methods: {},
    props: {
        tabGroups: Array,
        nsQuery: NamespaceQuery,
        dashboards: Object,
    }
};
</script>
