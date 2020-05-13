<template>
    <Bubble :hasContent="true" :hasSidebar="false">
        <template v-slot:header>
            <div class="items header-items">
                <span> {{ component.name }} </span>
                <span class="destroy icon" @click.stop="destroy">
                    <font-awesome-icon icon="trash"/>
                </span>
            </div>
        </template>
        <template v-slot:content>
            <div class="items content-items">
                <span class="content-type">
                    {{ component.type }}
                </span>
                <span class="content-icon icon" @click.stop="streamRequest">
                    <font-awesome-icon icon="exchange-alt"/>
                </span>
                <span class="content-icon icon" @click.stop="info">
                    <font-awesome-icon icon="question"/>
                </span>
            </div>
        </template>
    </Bubble>
</template>

<script>
import { Component } from 'telegraph'

import Bubble  from '../components/Bubble.vue'
import uuidv4 from 'uuid/v4';

// globally register the componentinfopage
import ComponentInfoPage from '../pages/ComponentInfoPage.vue'

import Vue from 'vue'
Vue.component('ComponentInfoPage', ComponentInfoPage);

export default {
    name: 'ComponentView',
    components : { Bubble },
    props: {
        component: Component
    },
    methods: {
        destroy() {
        },
        streamRequest() {

        },
        info() {
            // create the popup
            var popup = {
                name: 'Component Information',
                type: 'ComponentInfoPage',
                props: {},
                id: uuidv4(),
                root: null
            };
            this.$bubble('popup', popup);
        }
    }
}
</script>

<style scoped>
.items {
    display: flex;
    flex-direction: row;
    padding-right: 12px;
}
.content-items {
    justify-content: flex-start;
    padding-right: 7px;
}
.header-items {
    justify-content: space-between;
}
.icon {
    color: #5e6870;
    transition: color 0.3s;
}
.content-icon {
    font-size: 0.8rem;
    margin: 2px;
}
.content-icon:hover {
    color: #e5e5e5;
}
.destroy {
    font-size: 0.7rem;
    transition: color 0.3s;
    align-self: center;
}
.destroy:hover {
    color: #ed4949;
}

.content-type {
    color: #1c8ed7;
    width: 100%;
}
</style>