<template>
  <div class="grid">
    <grid-layout class="internal-grid"
            :layout="tiles"
            :col-num="16"
            :row-height="30"
            :is-draggable="true"
            :is-resizable="true"
            :vertical-compact="true"
            :margin="[10, 10]"
            :use-css-transforms="true">
        <grid-item v-for="item in tiles"
                   v-bind:key="item.id"
                   :x="item.x"
                   :y="item.y"
                   :w="item.w"
                   :h="item.h"
                   :i="item.i">
          {{item.i}}
        </grid-item>
    </grid-layout>
  </div>
</template>

<script>
import VueGridLayout from 'vue-grid-layout'

var GridLayout = VueGridLayout.GridLayout
var GridItem = VueGridLayout.GridItem

export default {
  name: 'grid',
  components: {
    GridLayout,
    GridItem
  },
  data: function () {
    return {
      tiles: [{
        id: 0,
        x: 0,
        y: 0,
        w: 2,
        h: 5,
        i: '0'
      }, {
        id: 1,
        x: 2,
        y: 0,
        w: 2,
        h: 2,
        i: '1'
      }]
    }
  }
}
</script>

<style scoped>
.internal-grid {
   width: 100%;
   height: 100%;
}
</style>
