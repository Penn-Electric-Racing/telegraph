<template>
  <Panel @close="$emit('delete')">
  </Panel>
</template>

<script>
  import Panel from '../components/Panel.vue'
  export default {
    name: 'Graph',
    components: { Panel }
  }
</script>