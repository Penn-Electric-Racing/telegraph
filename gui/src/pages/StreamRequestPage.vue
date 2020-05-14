<template>
    <div class="requests-page">
        <ScrollArea>
            <div class="requests-list">
                <div class="request" v-for="(r, i) in requests" :key="i">
                    <div class="params message">
                        <span class="outbound prefix">&lt;&lt;</span>
                        <JsonPretty :data="r.payload"/>
                    </div>

                    <div class="response message" v-for="(x, j) in r.responses" :key="j">
                        <span class="inbound prefix">&gt;&gt;</span> 
                        <JsonPretty :data="x"/>
                    </div>

                    <div class="message" v-if="r.req && r.req.isClosed">
                        <span class="inbound prefix">&gt;&gt;</span>
                        <span class="error">Closed</span>
                    </div>

                    <div class="message" v-if="!r.req">
                        <span class="error">Got no response!</span>
                    </div>
                </div>
            </div>
        </ScrollArea>
        <div class="requests-form">
            <TextField v-model="data" placeholder="Request JSON..." @submit="submit"/>
            <Button text="Send" class="request-submit-button" @click="submit"/>
        </div>
    </div>
</template>

<script>
import Table from '../components/Table.vue'
import TableRow from '../components/TableRow.vue'
import TableHeader from '../components/TableHeader.vue'
import TableData from '../components/TableData.vue'

import TextField from '../components/TextField.vue'
import Button from '../components/Button.vue'
import ScrollArea from '../components/ScrollArea.vue'

import JsonPretty from 'vue-json-pretty'
import Vue from 'vue'

export default {
    name: 'StreamRequestPage',
    components: {Table, TableRow, TableHeader, 
                 TableData, TextField, Button,
                 JsonPretty, ScrollArea},
    props: {
        resource: Object
    },
    data() {
        return {
            requests: [], // tuples of (Request, payload, responses)
            data: ""
        }
    },
    methods: {
        submit() {
            var payload = null;
            if (this.data != "") {
                try {
                    payload = JSON.parse(this.data);
                } catch (e) {
                    console.log(e);
                    return;
                }
            }
            // start asynchronous stream call
            (async () => {
                // if call was successful, add a request
                // entry
                try {
                    var req = await this.resource.request(payload);
                    var robj = Vue.observable({
                        req: req,
                        payload: payload,
                        responses: []
                    });
                    this.requests.push(robj);
                    if (req) {
                        req.received.add(r => robj.responses.push(r));
                        req.start();
                    }
                    this.data = "";
                } catch (e) {
                    console.log(e);
                }
            })();
        }
    }
}
</script>

<style scoped>
.requests-page {
    display: flex;
    flex-direction: column;
    align-items: stretch;
}
.requests-list {
    margin-bottom: 20px;
}
.message {
    display:flex;
    flex-direction: row;
}
.error {
    color: #ed4949;
}
.inbound {
    color: #d249ed;
}
.outbound {
    color: #1d81df;
}
.prefix {
    margin-right: 0.5rem;
}
.request-submit-button {
    margin-left: 0.5rem;
}
</style>