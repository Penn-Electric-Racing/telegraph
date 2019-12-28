<template>
  <div class="live-view">
    foo
    <div class="live-tree">
      <TreeView/>
    </div>
    <div class="connection-selector">
    </div>
  </div>
</template>

<script>
import TreeView from './TreeView.vue'

export default {
  name: 'LivePage',
  props: {
    client: Object
  },
  components: {TreeView}
}
</script>

<style>
</style>
