<template>
  <div class="tree-view">
    <TextField class="tree-search" v-model="filter" placeholder="Enter Filter..."/>

    <NodeView class="tree" :node="tree" v-if="tree != undefined"/>
    <div class="tree-placeholder" v-else>
      <p>{{ placeholder }}</p>
    </div>

  </div>
</template>

<script>
import { NodeQuery } from 'telegraph'

import NodeView from './NodeView.vue'
import TextField from '../components/TextField.vue';

export default {
  name: 'TreeView',
  components: { NodeView, TextField },
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
.tree-placeholder {
  border: 2px dashed #5E6870;
  border-radius: 16px;
  box-sizing: border-box;

  margin: 10px;
  width: calc(100% - 20px);
  height: calc(100% - 20px);

  display: flex;
  justify-content: center;
  align-items: center;

  color: #5E687033;
  font-size: 2rem;
}

.tree {
  margin: 5px;
}
.tree-placeholder p {
  text-align: center;
  margin: 5px;
}
.tree-view {
  display: flex;
  flex-direction: column;
}

</style>
