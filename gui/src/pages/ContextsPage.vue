<template>
    <div class="contexts-page">
        <ScrollArea>
            <div class="cotnexts">
                    <ContextView v-for="context in contexts" 
                                :key="context.uuid"
                                :context="context"/>
            </div>
        </ScrollArea>
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
import ContextView from '../views/ContextView.vue'
import Bubble from '../components/Bubble.vue'
import ScrollArea from '../components/ScrollArea.vue'

import CreatePage from './CreatePage.vue'

import { NamespaceQuery } from 'telegraph'

import uuidv4 from 'uuid/v4';
import Vue from 'vue'

Vue.component('CreatePage', CreatePage);

export default {
    name: 'ContextsPage',
    components: {ContextView, Bubble, ScrollArea},
    props: {
        nsQuery: NamespaceQuery
    },
    data: function() {
        return {
            contextCollection: null,
            contexts: []
        }
    },
    created() {
        if (this.nsQuery) {
            this.contextCollection = this.nsQuery.contexts.collect();
            this.contextCollection.added.add(x => this.contexts.push(x));
            this.contextCollection.removed.add(x => this.contexts.splice(this.contexts.indexOf(x), 1));
        } else {
            this.contextCollection = null;
        }
    },
    methods: {
        createNew() {
            // popup a creation dialogue
            this.$bubble('popup', {
                name: 'Create Context',
                type: 'CreatePage',
                id: uuidv4(),
                props: {resourceType:'Context'}
            });
        }
    },
    watch: {
        nsQuery() {
            if (this.nsQuery) {
                this.contextCollection = this.nsQuery.contexts.collect();
                this.contextCollection.added.add(x => this.contexts.push(x));
                this.contextCollection.removed.add(x => this.contexts.splice(this.contexts.indexOf(x), 1));
            } else {
                this.contextCollection = null;
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
