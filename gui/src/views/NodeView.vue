<template>
  <Bubble v-if="isGroup" class="node-bubble" 
                draggable hasContent :dragData="node">
    <template v-slot:header>
        {{ node.getName() }}
    </template>
    <template v-slot:content>
      <NodeView v-for="n in children" :node="n" :filter="filter" :key="n.getName()"/>
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
      let children = this.node.getChildren ? this.node.getChildren() : [];
      let f = [];
      if (!this.filter) {
        f = children;
      } else {
        let anyOf = this.filter.split(' ');
        for (let c of children) {
          var allow = false;
          for (let d of c.nodes()) {
            var path = '/' + d.path().join('.');
            for (let p of anyOf) {
              var parts = p.split('/');
              let d = '';
              if (parts.length > 0 && parts[0].length == 0) {
                parts.shift();
                d = '/';
              }
              d = d + parts.join('.');
              if (path.indexOf(d) >= 0) {
                allow = true;
                break;
              }
            }
            if (allow) break;
          }
          if (allow) f.push(c);
        }
      }
      return f;
    }
  },
  props: {
    node: Node,
    filter: {type: String, default: null}
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
