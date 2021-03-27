<template>
	<div class="tabpane">
        <component 
            v-bind:is="tabgroup.layout" 
            :key="tabgroup.id"
            :tabgroup="tabgroup"
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
import TabGroup from './TabGroup.vue';
import HorizontalSplitTabGroup from './HorizontalSplitTabGroup.vue';
import VerticalSplitTabGroup from './VerticalSplitTabGroup.vue';

import { NamespaceQuery } from 'telegraph';

export default {
	name: "TabPane",
    components: { TabGroup, HorizontalSplitTabGroup, VerticalSplitTabGroup },
    props: {
        tabgroup: Object,
        nsQuery: NamespaceQuery,
        dashboards: Object,
    }
};
</script>
