<template>
  <div id="app">
    <div class="container">
      <div>
        <Sidebar :expanded="sidebarExpanded"></Sidebar>
      </div>

      <nav class="burger">
        <Burger :expanded="sidebarExpanded"
                  v-on:collapse="collapseSidebar"
                  v-on:expand="expandSidebar"></Burger>
      </nav>

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

  background-color: #D4D4D4;
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
  margin-left: 0%;
  margin-right: 0;
  padding-top: 3rem;
  flex: 1;
  height: 100%;
  width: 100%;
}

.burger {
  position: absolute;
  padding: 0.5rem 0.8rem;
  z-index: 100;
}

</style>
