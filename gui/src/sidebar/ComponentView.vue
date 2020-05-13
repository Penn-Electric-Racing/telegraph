<template>
    <Bubble :header="component.name">
    </Bubble>
</template>

<script>

import { Component } from 'telegraph'

import Bubble  from '../components/Bubble.vue'

export default {
    name: 'ComponentView',
    components : { Bubble },
    props: {
        component: Component
    }
}

</script>