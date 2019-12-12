/* inspired by theme
  http://demo.adminbootstrap.com/right/1.0.0/index.html */
<template>
  <div id="app">
    <div id="header-container">
      <Burger v-on:toggle="toggleSidebar"></Burger>

      <TabSwitcher :tabs="dashboards" 
                   :activeId="activeDashboard" 
               v-on:selected="dashboardSelected"/>
    </div>

    <div id="content-container">
      <transition name="sidebar"> 
        <Sidebar class="sidebar" v-if="sidebarShowing"></Sidebar>
      </transition>

      <TabArea id="content-area">
        <TabPane :active="dashboard.id==activeDashboard"
          v-for="dashboard in dashboards" :key="dashboard.id">

          <Dashboard :name="dashboard.name" 
                     :store-location="dashboard.location"
                     :key="dashboard.id"/>

        </TabPane>
      </TabArea>
    </div>
  </div>
</template>

<script>
import TabSwitcher from './components/tabs/TabSwitcher.vue'
import TabArea from './components/tabs/TabArea.vue'
import TabPane from './components/tabs/TabPane.vue'

// interface components
import Sidebar from './sidebar/Sidebar.vue'
import Burger from './sidebar/Burger.vue'
import Dashboard from './dashboard/Dashboard.vue'

import {AppStore} from './app.js'
// import {DummyApp} from './dummy.js'

export default {
  name: 'App',

  data () {
    return {
      // store: createDummy(),

      sidebarShowing: false,
      workspace: null,


      // open dashboards 

      nextDashboardId: 0,
      // all the open dashboards, each
      // objects containing name, id, location (specifying load/store location)
      dashboards: [],
      activeDashboard: 0, // currently active dashboard id
    }
  },

  components: {
    TabSwitcher, TabArea, TabPane,

    Sidebar, Burger, Dashboard
  },

  methods: {
    dashboardSelected(id) {
      this.activeDashboard = id
    },
    createDashboard() {
      this.dashboards.push({name: 'Untitled', 
                location: null,
                id: this.nextDashboardId++})
    },
    toggleSidebar() {
      this.sidebarShowing = !this.sidebarShowing
    }
  },

  created() {
  },

  mounted() {
    this.workspace = this.$refs['workspace']
    setInterval(1000, function() { this.createDashboard('Untitled') })
  }

}
function createDummy() {
  return new DummyApp();
}
</script>

<style>
html, body {
  width: 100%;
  height: 100%;
  margin: 0;
}
#app {
  font-family: "Roboto", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  background-color: #343C42;
  width: 100%;
  height: 100%;

  display: flex;
  align-items: stretch;
  align-content: stretch;
  flex-direction: column;
}

#header-container {
  height: 3rem;
  width: 100%;
  background-color: #272c30;

  display: flex;
  align-items: stretch;
  align-content: stretch;
  flex-direction: row;
}

#content-container {
  width: 100%;
  height: 100%;

  display: flex;
  align-items: stretch;
  align-content: stretch;
  flex-direction: row;
}

#content-area {
  width: 100%;
  height: 100%;
}

.sidebar-enter-active, .sidebar-leave-active {
  transition: transform 0.25s ease-in-out;
}

.sidebar-enter, .sidebar-leave-to {
  transform: translateX(-100%)
}

.sidebar-enter-to, .sidebar-leave {
  transform: translateX(0%)
}

.sidebar  {
  height: 100%;
} 

</style>

