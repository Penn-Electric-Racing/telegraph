<template>
    <div class="verticalSplitParent">
        <component
            class="verticalSplitChild"

            v-for="tg in tabgroup.children"
            v-bind:is="tg.layout"
            :key="tg.id"
            :tabgroup="tg"
            :closeable="true"
            :editable="true"
            :draggable="true"
            :nsQuery="nsQuery"
            :dashboards="dashboards"

            :name="tg.layout"
            :style="{'height': (100 / tabgroup.children.length + '%')}"
        />
    </div>
</template>

<style scoped>
    .verticalSplitParent {
        display: flex;
        width: 100%;
        height: 100%;
        flex-flow: column nowrap;
    }

    .verticalSplitChild {
        width: 100%;
    }
</style>

<script>
import TabGroup from './TabGroup.vue';

import { NamespaceQuery } from 'telegraph';

export default {
	name: "VerticalSplitTabGroup",
    components: { 
        TabGroup, 
        HorizontalSplitTabGroup: () => import('./HorizontalSplitTabGroup.vue')
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
