<template>
    <Table class="context-table">
        <TableRow>
            <TableData>Name</TableData>
            <TableData>{{context ? context.name : ''}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>Type</TableData>
            <TableData>{{context ? context.type : ''}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>UUID</TableData>
            <TableData>{{context ? context.uuid : ''}}</TableData>
        </TableRow>
        <TableRow>
            <TableData>Parameters</TableData>
            <TableData>
                <JsonPretty :data="context ? context.params : undefined"/>
            </TableData>
        </TableRow>
    </Table>
</template>

<script>
import {Context} from 'telegraph'

import Table from '../components/Table.vue'
import TableRow from '../components/TableRow.vue'
import TableData from '../components/TableData.vue'
import JsonPretty from 'vue-json-pretty'

export default {
    name: 'ComponentInfoPage',
    components: {Table, TableRow, TableData, JsonPretty},
    props: {
        context: Context
    }
}
</script>

<style scoped>
.context-table {
    width: 100%;
}
</style>