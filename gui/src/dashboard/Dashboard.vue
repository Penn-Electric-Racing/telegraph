<template>
  <div class="dashboard" ref="dashboard">
    <grid-layout
        :layout.sync="layout"
        :col-num="numCols"
        :row-height="rowHeight">
      <grid-item v-for="item in layout"
                 class="tile"
                 :x="item.x"
                 :y="item.y"
                 :w="item.w"
                 :h="item.h"
                 :i="item.i"
                 dragIgnoreFrom=".noDrag"
                 :key="item.i">
        <component 
         v-bind:is="tileDataMap.get(item.i) ? tileDataMap.get(item.i).type : null"
               :id="item.i" :ns="ns"
               :dataMap="tileDataMap" @delete="remove(item.i)"/>
      </grid-item>
    </grid-layout>
  </div>
</template>

<script>
  import Vue from 'vue'
  import VueGridLayout from 'vue-grid-layout'
  import interact from 'interactjs'
  import { Namespace } from 'telegraph'

  import uuidv4 from 'uuid';

  // the valid types
  import Graph from './Graph.vue'
  import ControlPanel from './ControlPanel.vue'
  import Placeholder from './Placeholder.vue'

  export default {
    name: 'Dashboard',
    components: {ControlPanel, Graph, Placeholder,
      GridLayout: VueGridLayout.GridLayout, 
      GridItem: VueGridLayout.GridItem},
    props: {
      name: String,
      id: String,
      ns: Namespace
    },
    data: function() {
      return {
        numCols: 30,
        rowHeight: 30,
        dragOver: false,
        layout: [], // stores widget id, x, y, width, height
        tileDataMap: new Map() // map from widget id => widget data object
      };
    },
    methods: {
      remove(id) {
        this.tileDataMap.delete(id);
        this.layout.splice(this.layout.findIndex(x => x.i == id), 1);
      },
      drop(x, y, data) {
        var r = this.$refs['dashboard'].getBoundingClientRect();

        var col = Math.max(0, Math.round(x / (r.width/this.numCols)));
        var row = Math.max(0, Math.round(y / this.rowHeight));

        var tile = Vue.observable(
          { type: 'Placeholder',  
            ctx: data.getContext().getName(), 
            node: data.path() });

        var tileUUID = uuidv4();
        this.tileDataMap.set(tileUUID, tile);

        // data should be a node
        this.layout.push({i: tileUUID, x:col, y:row, w:4, h:4});
      }
    },
    mounted() {
      // listen for drop events
      interact(this.$refs['dashboard'])
        .dropzone({
          overlap: 'pointer',
          accept: '.node-bubble'
        })
        .on('dragenter', (event) => {
          this.dragOver = true;
        }).on('dragleave', (event) => {
          this.dragOver = false;
        }).on('drop', (event) => {
          var x = parseFloat(event.relatedTarget.getAttribute('data-x'));
          var y = parseFloat(event.relatedTarget.getAttribute('data-y'));

          var r = this.$refs['dashboard'].getBoundingClientRect();
          this.drop(x - r.left, y - r.top, event.interaction.data);
        });
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
  .drop-activated {
    opacity: 0.5;
  }
  .tile {
    background-color: #30363c;
    color: #88939C;
    border: 0px;
    border-radius: 4px;
    overflow: hidden;
  }
</style>
