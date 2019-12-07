<template>
  <div id="app">
    <div class="container">
      <Sidebar :expanded="sidebarExpanded"></Sidebar>
      <Burger :expanded="sidebarExpanded"
                v-on:collapse="collapseSidebar"
                v-on:expand="expandSidebar"></Burger>
      <div id="content-container">
        <TabbedView :headersLeftPad="sidebarExpanded ? '0px' : '60px'">
          <Tab name="Dashboard">
            <GridView>
              <TileView header="Example Tile">
                foo
              </TileView>
            </GridView>
          </Tab>
          <Tab name="Dashboard 2">
            <GridView>
              <TileView header="Example Tile 2">
                bar
              </TileView>
            </GridView>
          </Tab>
        </TabbedView>
      </div>
    </div>
  </div>
</template>

<script>
import TileView from './components/TileView.vue'
import GridView from './components/GridView.vue'
import TabbedView from './components/TabbedView.vue'
import Tab from './components/Tab.vue'
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
    TabbedView, Tab,
    TileView, GridView, Burger, Sidebar
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
  align-content: stretch;
  flex-direction: row;
  height: 100%;
  width: 100%;
}

#content-container {
  width: 100%;
  height: 100%;
  background-color: #30363c;
}

.burger {
  position: absolute;
  padding: 0.5rem 0.8rem;
  z-index: 1000;
}
</style>
