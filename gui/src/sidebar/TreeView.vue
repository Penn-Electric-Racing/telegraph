<template>
  <div class="root">
    <NodeView :node="tree.getRoot()"/>
  </div>
</template>

<script>
import { Tree } from '../../../js/lib/tree.mjs'

export default {
  name: 'TreeView',
  data () {
    return {
    }
  },

  props: {
    tree: Tree
  },
}
</script>

<style>

ul.tree-wiew > li{
  list-style-type: none;
  padding-bottom: 1rem;
  color: #FFFFFF;
}



</style>
