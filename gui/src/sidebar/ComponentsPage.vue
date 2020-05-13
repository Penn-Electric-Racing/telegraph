<template>
    <div class="components-page">
        <ComponentView v-for="component in components" 
                    :key="component.uuid"
                    :component="component"/>
    </div>
</template>

<script>
import ComponentView from './ComponentView.vue'

import { NamespaceQuery } from 'telegraph'

export default {
    name: 'ComponentsPage',
    components: {ComponentView},
    props: {
        nsQuery: NamespaceQuery
    },
    data: function() {
        return {
            componentCollection: null,
            components: []
        }
    },
    created() {
        if (this.nsQuery) {
            this.componentCollection = this.nsQuery.components.collect();
            console.log(this.nsQuery);
            this.componentCollection.added.add(x => this.components.push(x));
            this.componentCollection.removed.add(x => this.components.splice(this.components.indexOf(x), 1));
        } else {
            this.componentCollection = null;
        }
    },
    watch: {
        nsQuery() {
            if (this.nsQuery) {
                this.componentCollection = this.nsQuery.components.collect();
                this.componentCollection.added.add(x => this.components.push(x));
                this.componentCollection.removed.add(x => this.components.splice(this.components.indexOf(x), 1));
            } else {
                this.componentCollection = null;
            }
        }
    }
}
</script>

<style scoped>
    .components-page {
        height: 100%;
        width: 100%;
        display: flex;
        flex-direction: column;
        align-items: stretch;
    }
</style>
