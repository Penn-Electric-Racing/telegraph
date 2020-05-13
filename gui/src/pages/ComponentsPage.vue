<template>
    <div class="components-page">
        <div class="components">
            <ComponentView v-for="component in components" 
                        :key="component.uuid"
                        :component="component"/>
        </div>
        <Bubble @click="createNew" class="plus-bubble">
            <template v-slot:header>
                <div class="plus">
                    <font-awesome-icon icon="plus"/>
                </div>
            </template>
        </Bubble>
    </div>
</template>

<script>
import ComponentView from '../views/ComponentView.vue'
import Bubble from '../components/Bubble.vue'

import { NamespaceQuery } from 'telegraph'

export default {
    name: 'ComponentsPage',
    components: {ComponentView, Bubble},
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
            this.componentCollection.added.add(x => this.components.push(x));
            this.componentCollection.removed.add(x => this.components.splice(this.components.indexOf(x), 1));
        } else {
            this.componentCollection = null;
        }
    },
    methods: {
        createNew() {
            console.log("creating!");
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
    .components {
        width: 100%;
        display: flex;
        flex-direction: column;
        align-items: stretch;
    }
    .plus {
        font-size: 1.2rem;
        text-align: center;
    }
    .plus-bubble {
        transition: background-color 0.5s ease-in-out, color 0.5s ease-in-out;
    }
    .plus-bubble:hover {
        color: #1c8ed7;
    }
</style>
