<template>
  <div>
  <!-- <button v-on:click="count++">You clicked me {{ count }} times.</button> -->
  <ul class = "Tree-View">
    <li v-for="gro in group" v-bind:key="gro">
      {{ gro }}
    </li>
    <li v-for="act in action" v-bind:key="act">
      {{ act }}
    </li>
    <li v-for="vari in variable" v-bind:key="vari">
      {{ vari }}
    </li>
  </ul>

  <TreeView v-bind:Root = AMS></TreeView>
  </div>
</template>

<script>
// Group, Tree,  Type, Action
import { Group, Variable, Action } from '../../../js/lib/tree.mjs'
import TreeView from './TreeView.vue'

function parser (unknown) {
  var groupArray = []
  var actionArray = []
  var variableArray = []
  unknown.forEach(function (element) {
    if (element instanceof Group) {
      groupArray.push(element.getPretty())
    } else if (element instanceof Action) {
      actionArray.push(element.getPretty())
    } else if (element instanceof Variable) {
      variableArray.push(element.getPretty())
    }
  })
  return [groupArray, actionArray, variableArray]
}

export default {
  data () {
    return {
      group: parser(this.Root.getChildren())[0],
      action: parser(this.Root.getChildren())[1],
      variable: parser(this.Root.getChildren())[2],
      AMS: (parser(this.Root.getChildren())[0])[0]
    }
  },

  props: {
    Root: Group
  },

  methods: {
  },

  components: { TreeView }
}
</script>

<style>

ul.Tree-View > li{
  list-style-type: none;
  padding-bottom: 1rem;
  color: #FFFFFF;
}

</style>
