<template>
  <div class="tree-view">
    <TextField class="tree-search" v-model="filter" placeholder="Enter Filter..."/>
    <ScrollArea v-if="tree != undefined">
      <NodeView class="tree" :node="tree"/>
    </ScrollArea>
    <Placeholder v-else :text="placeholder"/>
  </div>
</template>

<script>
import { NodeQuery } from 'telegraph'

import NodeView from './NodeView.vue'
import TextField from '../components/TextField.vue';
import Placeholder from '../components/Placeholder.vue';
import ScrollArea from '../components/ScrollArea.vue';

export default {
  name: 'TreeView',
  components: { NodeView, TextField, Placeholder, ScrollArea },
  data () {
    return {
      filter: '',
      tree: null
    }
  },
  props: {
    treeQuery: NodeQuery,
    placeholder: { type: String, default: "Not Found" }
  },
  created() {
    if (this.treeQuery) {
      this.tree = this.treeQuery.current;
      this.treeQuery.updated.add((newTree) => { this.tree = newTree });
    }
  },
  watch: {
    treeQuery() {
      this.tree = this.treeQuery.current;
      this.treeQuery.updated.add((newTree) => { this.tree = newTree });
    }
  }
}
</script>

<style scoped>
.tree {
  margin: 5px;
  min-height: 0px;
}
.tree-view {
  display: flex;
  flex-direction: column;
}

</style>
