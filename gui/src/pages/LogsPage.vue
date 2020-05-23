<template>
    <div class="logs-container">
        <ScrollArea>
            <div class="log-list">
                <div v-for="(dash, id) in dashboards" 
                        class="log-item" :key="id">
                    {{ dash.info.name }}
                    <div class="icons">
                        <font-awesome-icon class="icon open" @click="open(id)" icon="folder-open"/>
                        <font-awesome-icon class="icon destroy" @click="destroy(id)" icon="trash"/>
                    </div>
                </div>
            </div>
        </ScrollArea>
    </div>
</template>

<script>
import { NamespaceQuery } from 'telegraph'
import ScrollArea from '../components/ScrollArea.vue'
import Vue from 'vue'
export default {
    name: 'LogsPage',
    components: { ScrollArea },
    props: {
        nsQuery: NamespaceQuery,
        dashboards: Object
    },
    methods: {
        open(id) {
        },
        destroy(id) {
        }
    }
}
</script>

<style scoped>
.log-list {
    padding: 5px;
    display: flex;
    flex-direction: column;
    align-items: stretch;
}
.log-item {
    margin: 3px;
    padding: 3px;

    display: flex;
    flex-direction: row;
    align-items: center;
    justify-content: space-between;

    box-shadow: 0px 0px 10px #272c30;
    background-color: #272c30;
    border-radius: 3px;
    color: #5e6870;
}
.icon {
    margin: 3px;
    transition: color 0.3s ease-in-out;
    font-size: 0.8rem;
}
.icon.open:hover {
    color: #ffffff;
}
.icon.destroy:hover {
    color: #ed4949;
}
</style>