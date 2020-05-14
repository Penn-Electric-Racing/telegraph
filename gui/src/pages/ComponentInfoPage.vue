<template>
    <Table class="component-table">
        <TableRow>
            <TableData>Name</TableData>
            <TableData>{{component.name}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>Type</TableData>
            <TableData>{{component.type}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>UUID</TableData>
            <TableData>{{component.uuid}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>Parameters</TableData>
            <TableData>
                <JsonPretty :data="component.params"/>
            </TableData>
        </TableRow>
    </Table>
</template>

<script>
import {Component} from 'telegraph'

import Table from '../components/Table.vue'
import TableRow from '../components/TableRow.vue'
import TableData from '../components/TableData.vue'
import JsonPretty from 'vue-json-pretty'

export default {
    name: 'ComponentInfoPage',
    components: {Table, TableRow, TableData, JsonPretty},
    props: {
        component: Component
    }
}
</script>

<style scoped>
.component-table {
    width: 100%;
}
</style>