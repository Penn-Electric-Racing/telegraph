<template>
  <div v-if="isGroup">
    {{ node.getName() }}
    <ul class="tree-view">
      <li v-for="n in children" v-bind:key="n.getName()">
        <TreeView v-bind:node="n"/>
      </li>
    </ul>
  </div>

  <div v-else-if="isAction">
    {{ node.getName() }}
  </div>

  <div v-else-if="isVariable">
    {{ node.getName() }}
  </div>

  <div v-else-if="isStream">
    {{ node.getName() }}
  </div>
</template>

<script>
// Group, Tree,  Type, Action
import { Node, Group, Variable, Action, Stream } from '../../../../js/lib/tree.mjs'

export default {
  name: 'TreeView',
  data () {
    return {
      children: [],
      bindings: []
    }
  },

  computed: {
    isGroup: function () { return this.node instanceof Group },
    isAction: function () { return this.node instanceof Action },
    isVariable: function () { return this.node instanceof Variable },
    isStream: function () { return this.node instanceof Stream }
  },

  props: {
    node: Node
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
