<template>
  <div class="tree-menu">
    <div>{{ label }}</div>
    <tree-menu 
      v-for="node in nodes" 
      :nodes="node.nodes" 
      :label="node.label"
      :depth="0"
      v-bind:key= "node"
    >
    </tree-menu>
  </div>
</template>
<script>
  export default { 
    props: [ 'label', 'nodes' ],
    name: 'tree-menu',
    computed: {
      indent() {
        return { transform: `translate(${this.depth * 50}px)` }
      }
    }
  }
</script>