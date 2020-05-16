<template>
  <Bubble v-if="isGroup" class="node-bubble" 
                draggable hasContent :dragData="node">
    <template v-slot:header>
        {{ node.getName() }}
    </template>
    <template v-slot:content>
      <NodeView v-for="n in children" :node="n" :key="n.getName()"/>
    </template>
  </Bubble>

  <Bubble v-else-if="isAction" class="node-bubble" 
                    :draggable="true" :dragData="node">
    <template v-slot:header>
        {{ node.getName() }}
    </template>
  </Bubble>
  <Bubble v-else-if="isVariable" class="node-bubble" 
                    :draggable="true" :dragData="node">
    <template v-slot:header>
        {{ node.getName() }}
    </template>
  </Bubble>
</template>

<script>
import { Node, Group, Variable, Action } from 'telegraph'
import Bubble from '../components/Bubble.vue'

export default {
  name: 'NodeView',
  components: { Bubble },
  computed: {
    isGroup() { return this.node instanceof Group },
    isAction() { return this.node instanceof Action },
    isVariable() { return this.node instanceof Variable },
    children() {
      return this.node.getChildren ? this.node.getChildren() : [];
    }
  },
  props: {
    node: Node
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
