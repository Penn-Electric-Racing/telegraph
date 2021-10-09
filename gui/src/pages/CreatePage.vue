<template>
    <div class="create-page">
        <Form>
            <FormItem label="Name">
                <TextField v-model="name" placeholder=""
                :inputValue="formName" @input="setName"/>
            </FormItem>
            <FormItem label="Type">
                <TextField v-model="type" placeholder=""
                :inputValue="formType" @input="setType"/>
            </FormItem>
            <FormItem label="Parameters">
                <TextField v-model="params" placeholder=""
                :inputValue ="formParams" @input="setParams"/>
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
import { mapGetters, mapSetters } from "vuex";

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
            params: "",
        }
    },
    computed: {
        ...mapGetters("popup", {
            formName: "getName",
            formType: "getType",
            formParams: "getParameters"
        }),
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
        },
        setName(newName) {
            this.$store.dispatch("popup/editName", newName);
        },
        setType(newType) {
            this.$store.dispatch("popup/editType", newType);
        },
        setParams(newParams) {
            this.$store.dispatch("popup/editParams", newParams);
        }
    }
}
</script>

<style scoped>
.label {
    padding-right: 1rem;
}
</style>