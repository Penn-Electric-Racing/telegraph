<template>
  <div id="app">
    <div class="header">
      <nav class="burger">
        <Burger :expanded="sidebarExpanded"
          v-on:collapse="collapseSidebar"
          v-on:expand="expandSidebar"></Burger>
      </nav>
    </div>

    <div class="container">
      <div>
        <Sidebar :expanded="sidebarExpanded"></Sidebar>
      </div>

      <div id="graph-grid">
        <grid>
          <tile header="Example Tile"/>
        </grid>
      </div>

    </div>
  </div>
</template>

<script>
import Tile from './components/Tile.vue'
import Grid from './components/Grid.vue'
import Burger from './components/menu/Burger.vue'
import Sidebar from './components/menu/Sidebar.vue'

export default {
  name: 'app',

  data () {
    return {
      sidebarExpanded: false
    }
  },

  components: {
    Tile, Grid, Burger, Sidebar
  },

  methods: {
    expandSidebar: function () {
      this.sidebarExpanded = true
    },
    collapseSidebar: function () {
      this.sidebarExpanded = false
    }
  }

}

</script>

<style>

html, body {
  height: 100%;
  margin: 0;
}

#app {
  font-family: "Poppins", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  background-color: #343C42;
  position: fixed;

  width: 100%;
  height: 100%;
}

.container {
  display: flex;
  align-items: stretch;
  flex-direction: row;
}

#graph-grid {
  position: relative;
  flex: 1;
  width: 100%;
}

#board-1 {
  background-color: blue;
  width: 10rem;
  color:white;
}

#board-2 {
  background-color: black;
  width: 10rem;
  color:white;
}

.burger {
  position: absolute;
  padding: 0.5rem 0.8rem;
  z-index: 1000;
}

.header {
  width: 100%;
  height: 3rem;
  background-color: #272c30;
  z-index: 1000;
}
</style>
