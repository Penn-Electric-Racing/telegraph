<template>
  <div class="tree-view">
    <TextField class="tree-search" v-model="filter" placeholder="Enter Filter..."/>

    <NodeView :node="tree.getRoot()" v-if="tree != undefined"/>

    <div class="tree-placeholder" v-else>
      <p>{{ placeholder }}</p>
    </div>

  </div>
</template>

<script>
import { Tree } from '@telegraph/core'

import TextField from '../components/TextField.vue';

export default {
  name: 'TreeView',
  components: { TextField },
  data () {
    return {
      filter: ''
    }
  },

  props: {
    tree: Tree,
    placeholder: { type: String, default: "Not Found" }
  },
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

.tree-placeholder p {
  text-align: center;
  margin: 5px;
}
.tree-view {
  display: flex;
  flex-direction: column;
}

</style>
