<template>
  <div class="tree-view">
    <TextField class="tree-search" v-model="filter" placeholder="Enter Filter..."/>
    <ScrollArea v-if="tree != undefined">
      <div>
        <NodeView class="tree" :node="tree" 
          :filter="filter" :showRecord="showRecord"/>
      </div>
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
    treeQuery: {NodeQuery, default: null},
    treeRoot: {Object, default: null},
    showRecord: {type: Boolean, default: false },
    placeholder: { type: String, default: "Not Found" }
  },
  created() {
    if (this.treeQuery) {
      this.tree = this.treeQuery.current;
      this.treeQuery.updated.add((newTree) => { this.tree = newTree });
    }
    if (this.treeRoot) {
      this.tree = this.treeRoot;
    }
  },
  watch: {
    treeQuery() {
      this.tree = this.treeQuery.current;
      this.treeQuery.updated.add((newTree) => { this.tree = newTree });
    },
    treeRoot() {
      this.tree = this.treeRoot;
    }
  }
}
</script>

<style scoped>
.tree {
  display: block;
  margin: 5px;
}
.tree-view {
  display: flex;
  flex-direction: column;
}

</style>
