/* inspired by theme
  http://demo.adminbootstrap.com/right/1.0.0/index.html */
<template>
  <div id="app">
    <!-- TODO: Instead of making the sidebar adjust size to its content maybe
        have the sidebar be draggable-resizable so you can choose how large you want it
        and the content in the sidebar can expand/show more things 
        (small picture graphs, etc) if you make it bigger -->

    <div id="header-container">
      <Burger :expanded="sidebarShowing" @toggle="toggleSidebar"
         :style="{minWidth: sidebarWidth > 0 ? sidebarWidth + 'px' : null}"></Burger>

      <TabSwitcher :tabs="tabs" 
                   :active="activeTab" 
                   :closeable="true" :editable="true" :draggable="true"
                   @selected="selectTab"
                   @closed="closeTab"
                   @renamed="renameTab"/>
      <div class="header-button-container">
        <FlatButton id="new-tab" icon="plus" @click="newTab"/>
      </div>
      <div class="header-spacer"/>
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
          <div v-show="activeSidebar=='dashboards'">
            Saved Dashboards
          </div>
          <div v-show="activeSidebar=='logs'">
            Logs
          </div>
          <div v-show="activeSidebar=='contexts'">
            Contexts
          </div>
          <ComponentsPage v-show="activeSidebar=='components'" :nsQuery="nsQuery"/>
          <LivePage v-show="activeSidebar=='live'" :nsQuery="nsQuery"/>
        </TabArea>
      </div>

      <TabArea id="content-area">
        <component :is="tab.type" 
                   :name="tab.name" 
                   :id="tab.id"
                   :nsQuery="nsQuery"
                   :key="tab.id"
                   @renamed="(name) => {renameTab(tab.id, name)}"
                   v-for="tab in loadedTabs"
                   v-show="tab.id==activeTab"/>
      </TabArea>
    </div>
  </div>
</template>

<script>
import TabSwitcher from './components/TabSwitcher.vue'
import TabArea from './components/TabArea.vue'

import FlatButton from './components/FlatButton.vue'

import LivePage from './sidebar/LivePage.vue'
import ComponentsPage from './sidebar/ComponentsPage.vue'

// interface components
import Burger from './sidebar/Burger.vue'
import Dashboard from './dashboard/Dashboard.vue'

import uuidv4 from 'uuid/v4';

import { Client, NamespaceQuery } from 'telegraph'

export default {
  name: 'App',
  data () {
    var namespace = new Client();
    return {
      sidebarShowing: true,
      sidebarWidth: null,
      sidebarHeaders: [
                       {"icon":"wifi", "id": "live"}, 
                       {"icon":"columns", "id": "dashboards"}, 
                       {"icon":"edit", "id": "logs"},
                       {"icon":"box", "id": "contexts"}, 
                       {"icon":"tasks", "id": "components"}, 
                       {"icon":"cogs", "id": "settings"}, 
                      ], 
      activeSidebar: "live",

      // each tab must have a type, a name, and an id
      // the name and id will be passed to the component created
      // according to the type specified
      //
      // tabs can also have the optional offload=true/false
      // which, if true will destroy the component when not shown
      tabs: [],
      activeTab: null, // the active tab ID

      namespace: namespace,
      nsQuery: new NamespaceQuery(),
      relay: null
    }
  },

  computed : {
    loadedTabs() {
      let loaded = [];
      for (let t of this.tabs) {
        if (t == this.activeTab || !t.offloaded) {
          loaded.push(t);
        }
      }
      return loaded;
    }
  },

  components: {
    TabSwitcher, TabArea,
    LivePage, ComponentsPage,
    FlatButton,

    Burger, Dashboard
  },

  methods: {
    selectSidebar(id) {
      this.activeSidebar = id;
    },
    selectTab(id) {
      this.activeTab = id
    },
    closeTab(id) {
      this.tabs.splice(this.tabs.findIndex((tab) => tab.id == id), 1);
      if (this.activeTab == id) {
        this.activeTab = null;
        if (this.tabs.length > 0) this.activeTab = this.tabs[0].id;
      }
    },
    renameTab(id, name) {
      for (let t of this.tabs) {
        if (t.id == id) {
          t.name = name;
          break;
        }
      }
    },

    newTab() {
      var id = uuidv4();
      this.tabs.push({type: 'Dashboard',
                      name: 'Untitled', 
                      id: id });
      if (this.activeTab == null) this.activeTab = id;
    },

    toggleSidebar() {
      this.sidebarShowing = !this.sidebarShowing
      if (!this.sidebarShowing) this.sidebarWidth = 0;
      else this.$nextTick(() => { this.sidebarWidth = this.$refs['sidebar'].offsetWidth});
    },

    async run() {
      while (true) {
        // connect with the client
        try {
          await this.namespace.connect('ws://localhost:8081');
          this.nsQuery.update(this.namespace);
          await this.namespace.wait(); // wait until done
          this.nsQuery.update(null); // set namespace to null
        } catch (e) {
          console.log('failed to connect, retrying...');
          console.log(e)
        }
        // try and reconnect every 5 seconds after losing connection
        await new Promise((res, rej) => setTimeout(res, 5000));
      }
    }
  },

  created() {
    this.run(); // will launch connection...

    // create a new dashboard
    this.newTab();
  },

  mounted() {
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
  color: #88939C;
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

.header-button-container {
  color: #5e6870;
  display: flex;
  align-items: center;
  align-content: center;
  padding: 5px;
}

.header-spacer {
  width: 100%;
}

#new-tab:hover {
  color: #1c8ed7;
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
  min-width: 220px;
  max-width: 220px;
  background-color: #30363c;

  display: flex;
  align-items: stretch;
  align-content: stretch;
  flex-direction: column;
} 

#sidebar-area {
  width: 100%;
  height: 100%;
  box-sizing: border-box;
}

</style>
