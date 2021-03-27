<template>
    <div class="horizontalSplitParent">
        <component
            class="horizontalSplitChild"

            v-for="tg in tabgroup.children"
            v-bind:is="tg.layout"
            :key="tg.id"
            :tabgroup="tg"
            :closeable="true"
            :editable="true"
            :draggable="true"
            :nsQuery="nsQuery"
            :dashboards="dashboards"

            :style="{'width': (100 / tabgroup.children.length + '%')}"
        />
    </div>
</template>

<style scoped>
    .horizontalSplitParent {
        display: flex;
        height: 100%;
        width: 100%;
        flex-flow: row nowrap;
    }

    .horizontalSplitChild {
        height: 100%;
    }
</style>

<script>
import TabGroup from './TabGroup.vue';

import { NamespaceQuery } from 'telegraph';

export default {
	name: "HorizontalSplitTabGroup",
    components: {
        TabGroup,
        VerticalSplitTabGroup: () => import('./VerticalSplitTabGroup.vue')
    },
    props: {
		tabgroup: Object,
        nsQuery: NamespaceQuery,
        dashboards: Object,
		closeable: { type: Boolean, default: false },
		editable: { type: Boolean, default: false },
		draggable: { type: Boolean, default: false },
	}
};
</script>
