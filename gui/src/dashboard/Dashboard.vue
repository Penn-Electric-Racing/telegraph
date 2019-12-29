<template>
  <div class="dashboard">
    <grid-layout
        :layout.sync="layout"
        :col-num="12"
        :row-height="30">
      <grid-item v-for="item in layout"
                 :x="item.x"
                 :y="item.y"
                 :w="item.w"
                 :h="item.h"
                 :i="item.i"
                 dragAllowFrom=".tile-header"
                 :key="item.i">
        <Tile :id="item.i" :dashboardId="id"/>
      </grid-item>
    </grid-layout>
  </div>
</template>

<script>
  import Tile from './Tile.vue'
  import VueGridLayout from 'vue-grid-layout'
  export default {
    name: 'Dashboard',
    components: {Tile: Tile, 
      GridLayout: VueGridLayout.GridLayout, 
      GridItem: VueGridLayout.GridItem},
    props: {
      name: String,
      id: String
    },
    data: function() {
      return {
        layout: [], // stores widget id, x, y, width, height
      };
    },
    created() {
      this.layout.push({i: 0, x:0, y:0, w:2, h: 2})
      this.layout.push({i: 1, x:4, y:0, w:2, h: 2})
    }
  }
</script>

<style>
</style>

<style scoped>
  .dashboard >>> .vue-grid-placeholder {
    background-color: #1C8ED733;
    border: 2px solid #1C8ED7;
    border-radius: 3px;
    opacity: 1;
  }
  .dashboard {
    width: 100%;
    height: 100%;
  }
</style>
