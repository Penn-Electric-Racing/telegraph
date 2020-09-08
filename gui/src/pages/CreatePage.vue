<template>
    <div class="create-page">
        <Form>
            <FormItem label="Name">
                <TextField v-model="name" placeholder=""/>
            </FormItem>
            <FormItem label="Type">
                <TextField v-model="type" placeholder=""/>
            </FormItem>
            <FormItem label="Parameters">
                <TextField v-model="params" placeholder=""/>
            </FormItem>
            <FormItem alignContent="center">
                <Button text="Create" @click="create"/>
            </FormItem>
        </Form>
    </div>
</template>

<script>
import Form from '../components/Form.vue'
import FormItem from '../components/FormItem.vue'

import TextField from '../components/TextField.vue'
import Button from '../components/Button.vue'

import { NamespaceQuery } from 'telegraph'

export default {
    name: 'CreatePage',
    components: {Form, FormItem,
                TextField, Button},
    props: {
        nsQuery: NamespaceQuery
    },
    data() {
        return {
            name: "",
            type: "",
            params: ""
        }
    },
    methods: {
        async create() {
            if (this.name.length == 0 || this.type.length == 0);
            if (!this.nsQuery || !this.nsQuery.current) {
                return;
            }
            var params = null;
            if (this.params.length > 0) {
                try {
                    params = JSON.parse(this.params);
                } catch (e) {
                    console.log(e);
                    return;
                }
            }
            var ns = this.nsQuery.current;
            await ns.create(this.name, this.type, params);
            this.$bubble('close');
        }
    }
}
</script>

<style scoped>
.label {
    padding-right: 1rem;
}
</style>