/* inspired by theme
  http://demo.adminbootstrap.com/right/1.0.0/index.html */
<template>
  <div id="app">
    <div id="header-container">
      <Burger :expanded="sidebarShowing" v-on:toggle="toggleSidebar"
         :style="{minWidth: sidebarWidth + 'px'}"></Burger>

      <TabSwitcher :tabs="dashboards" 
                   :active="activeDashboard" 
                   :closeable="true"
                   @selected="selectDashboard"
                   @closed="closeDashboard"/>
    </div>

    <div id="content-container">
      <div id="sidebar" v-show="sidebarShowing" ref="sidebar">
        <div id="sidebar-header">
          <TabSwitcher :tabs="sidebarHeaders" :active="activeSidebar" 
                       @selected="selectSidebar"/>
        </div>
        <TabArea id="sidebar-area">
          <div v-show="activeSidebar=='settings'">
            Settings
          </div>
          <div v-show="activeSidebar=='saved'">
            Load Dashboard 
          </div>
          <div v-show="activeSidebar=='logs'">
            Logs
          </div>
          <LivePage v-show="activeSidebar=='live'" :client="client"/>
        </TabArea>
      </div>

      <TabArea id="content-area">
        <Dashboard :name="dashboard.name" 
                   :store-location="dashboard.location"
                   :key="dashboard.id"
                   v-for="dashboard in dashboards"
                   v-show="dashboard.id==activeDashboard"/>
      </TabArea>
    </div>
  </div>
</template>

<script>
import TabSwitcher from './components/TabSwitcher.vue'
import TabArea from './components/TabArea.vue'
import TabPane from './components/TabPane.vue'

import LivePage from './sidebar/LivePage.vue'

// interface components
import Burger from './sidebar/Burger.vue'
import Dashboard from './dashboard/Dashboard.vue'

import uuidv4 from 'uuid/v4';

export default {
  name: 'App',
  data () {
    return {
      // store: createDummy(),

      sidebarShowing: true,
      sidebarWidth: null,
      sidebarHeaders: [{"icon":"wifi", "id": "live"}, 
                       {"icon":"cogs", "id": "settings"}, 
                       {"icon":"folder-open", "id": "saved"}, 
                       {"icon":"edit", "id": "logs"}], 
      activeSidebar: "live",

      dashboards: [],
      activeDashboard: null,
      client: null
    }
  },

  components: {
    TabSwitcher, TabArea, TabPane,
    LivePage,

    Burger, Dashboard
  },

  methods: {
    selectSidebar(id) {
      this.activeSidebar = id;
    },
    selectDashboard(id) {
      this.activeDashboard = id
    },
    closeDashboard(id) {
    },
    createDashboard() {
      this.dashboards.push({name: 'Untitled', 
                location: null,
                id: uuidv4() });
    },
    toggleSidebar() {
      this.sidebarShowing = !this.sidebarShowing
      if (!this.sidebarShowing) this.sidebarWidth = 0;
      else this.$nextTick(() => { this.sidebarWidth = this.$refs['sidebar'].offsetWidth});
    }
  },

  created() {
  },

  mounted() {
    this.createDashboard();
    this.createDashboard();
    //setInterval(() => { this.createDashboard('Untitled') }, 20000)
    new ResizeObserver(() => {
      if (this.$refs['sidebar']) 
        this.sidebarWidth = this.$refs['sidebar'].offsetWidth;
    }).observe(this.$refs['sidebar']);
  }

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

#sidebar  {
  height: 100%;
  background-color: #30363c;
  transition: width 1s ease-in-out;
} 

.hidden {
  display: none;
}

</style>
