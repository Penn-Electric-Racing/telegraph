<template>
  <div class="root">
  </div>
</template>

<script>
import { Tree } from '../../../js/lib/tree.mjs'

export default {
  name: 'TreeView',
  data () {
    return {
      children: [],
      bindings: []
    }
  },

  computed: {
  },

  props: {
    tree: Tree
  },

  created: function () {
    if (this.node instanceof Group) {
      this.children.push(...this.node.getChildren())

      this.bindings.push(this.node.onAddChild.add(function (c) {
        this.children.push(c)
      }))

      this.bindings.push(this.node.onRemoveChild.add(function (c) {
        this.children.push(c)
      }))
    }
  },

  destroyed: function () {
    for (let b of this.bindings) {
      b.detach()
    }
  }
}
</script>

<style>

ul.tree-wiew > li{
  list-style-type: none;
  padding-bottom: 1rem;
  color: #FFFFFF;
}



</style>
