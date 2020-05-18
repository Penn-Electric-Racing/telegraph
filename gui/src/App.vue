/* inspired by theme
  http://demo.adminbootstrap.com/right/1.0.0/index.html */
<template>
  <div id="app">
    <div id="popup-pane" :style="{zIndex: numPopups > 0 ? 1 : -1}">
        <Popup v-for="(popup, idx) in popups" :name="popup.name" :id="popup.id" :key="popup.id"
                @popup="newPopup" @newtab="newTab" @close="() => {closePopup(popup.id)}"
                @focus="() => { focusPopup(popup.id) }">
          <component :is="popup.type" 
                    :name="popup.name" 
                    :id="popup.id"
                    :nsQuery="nsQuery"
                    :key="popup.id"
                    v-bind="popup.props"
                    :z-index="idx"
                    @renamed="(name) => {renamePopup(popup.id, name)}"/>
        </Popup>
    </div>
    <div id="app-content">
      <!-- TODO: Instead of making the sidebar adjust size to its content maybe
          have the sidebar be draggable-resizable so you can choose how large you want it
          and the content in the sidebar can expand/show more things 
          (small picture graphs, etc) if you make it bigger -->

      <div id="header-container">
        <Burger :expanded="sidebarShowing" @toggle="toggleSidebar"
          :style="{minWidth: sidebarWidth > 0 ? sidebarWidth + 'px' : null}"></Burger>
        <div class="header-tabs">
          <TabSwitcher :tabs="tabs" 
                      :active="activeTab" 
                      :closeable="true" :editable="true" :draggable="true"
                      @selected="selectTab"
                      @closed="closeTab"
                      @renamed="renameTab"/>
        </div>
        <div class="header-button-container">
          <FlatButton id="new-tab" icon="plus" @click="newDashboard"/>
        </div>
      </div>

      <div id="content-container">
        <div id="sidebar" v-show="sidebarShowing" ref="sidebar">
          <div id="sidebar-header">
            <TabSwitcher :tabs="sidebarHeaders" :active="activeSidebar" 
                        @selected="selectSidebar"/>
          </div>
          <TabArea id="sidebar-area" @newtab="newTab" @closetab="closeTab"
                                     @popup="newPopup">
            <div v-show="activeSidebar=='settings'">
              Settings
            </div>
            <DashboardsPage v-show="activeSidebar=='dashboards'" 
                :nsQuery="nsQuery" :dashboards="dashboards"/>
            <div v-show="activeSidebar=='logs'">
              Logs
            </div>
            <ContextsPage v-show="activeSidebar=='contexts'" :nsQuery="nsQuery"/>
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
                    v-bind="tab.props"
                    @popup="newPopup"
                    @newtab="newTab"
                    @renamed="(name) => {renameTab(tab.id, name)}"
                    @close="() => {closeTab(tab.id)}"
                    v-for="tab in loadedTabs"
                    v-show="tab.id==activeTab"/>
        </TabArea>
      </div>
    </div>
  </div>
</template>

<script>
import TabSwitcher from './components/TabSwitcher.vue'
import TabArea from './components/TabArea.vue'
import Popup from './components/Popup.vue'

import FlatButton from './components/FlatButton.vue'

import LivePage from './pages/LivePage.vue'
import ComponentsPage from './pages/ComponentsPage.vue'
import ContextsPage from './pages/ContextsPage.vue'
import DashboardsPage from './pages/DashboardsPage.vue'

// interface components
import Burger from './Burger.vue'
import Dashboard from './dashboard/Dashboard.vue'

import uuidv4 from 'uuid/v4';
import Vue from 'vue';

import { Client, NamespaceQuery } from 'telegraph'

export default {
  name: 'App',
  components: {
    TabSwitcher, TabArea,
    LivePage, ComponentsPage,
    ContextsPage, DashboardsPage,
    FlatButton, Popup,

    Burger, Dashboard
  },
  data () {
    var namespace = new Client();
    return {
      popups: [],
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

      dashboards: {}, // uuid -> data map

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
    },
    numPopups() {
      return this.popups.length;
    }
  },


  methods: {
    selectSidebar(id) {
      this.activeSidebar = id;
    },
    toggleSidebar() {
      this.sidebarShowing = !this.sidebarShowing
      if (!this.sidebarShowing) this.sidebarWidth = 0;
      else this.$nextTick(() => { this.sidebarWidth = this.$refs['sidebar'].offsetWidth});
    },

    newDashboard() {
      var dashData = Vue.observable({
        widgets:{}, layout:[],
        info: {name: 'Untitled'}
      });
      var id = uuidv4();
      this.tabs.push(Vue.observable({type: 'Dashboard',
                      name: 'Untitled', 
                      props: {data:dashData},
                      id: id }));
      if (this.activeTab == null) this.activeTab = id;
    },

    newTab(obj) {
      if (!obj || !obj.id) return;
      for (let t of this.tabs) {
        if (t.id == obj.id) return;
      }
      this.tabs.push(Vue.observable(obj));
      if (this.activeTab == null) this.activeTab = obj.id;
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
          // TODO: right now we assume all tabs are dashboards
          t.name = name;
          t.props.data.info.name = name;
          if (!this.dashboards[t.id]) {
            this.$set(this.dashboards, t.id, t.props.data);
          }
          break;
        }
      }
    },

    newPopup(popup) {
      if (!popup || !popup.id) return;
      for (let p of this.popups) {
        if (p.id == popup.id) return;
      }
      this.popups.push(popup);
    },

    closePopup(id) {
      for (let [index, p] of this.popups.entries()) {
        if (p.id == id) {
          this.popups.splice(index, 1);
          break;
        }
      }
    },

    focusPopup(id) {
      let popup = null;
      for (let [index, p] of this.popups.entries()) {
        if (p.id == id) {
          popup = p;
          this.popups.splice(index, 1);
          break;
        }
      }
      if (popup) {
        this.popups.push(popup);
      }
    },

    renamePopup(id, newName) {

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
    },

    saveDashboards(event) {
      delete event['returnValue']

      try {
        var j = JSON.stringify(this.dashboards);
        console.log('saving', j);
        if (window) {
          window.localStorage.setItem('dashboards', j);
        }
      } catch (e) {}
    }
  },

  created() {
    this.run(); // will launch connection...
    if (window) {
      var stored = window.localStorage.getItem('dashboards');
      if (stored) {
        var j = JSON.parse(stored);
        this.dashboards = Vue.observable(JSON.parse(stored));
        console.log('loaded:', j);
      }
      window.addEventListener('beforeunload', this.saveDashboards);
    }
    // create a new dashboard
    this.newTab();
  },
  destroyed() {
    window.removeEventListener('beforeunload', this.saveDashboards);
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
  min-width: 0;
  min-height: 0;
  overflow: hidden;
}
#popup-pane {
  position: absolute;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background-color: rgba(0, 0, 0, 0.3);
}
#popup-pane.darkened {
  z-index: 1000000;
}
#app {
  width: 100%;
  height: 100%;
  font-family: "Roboto", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  background-color: #343C42;
  color: #88939C;
}
#app-content {
  z-index: 0;

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
  flex-direction: row;
}

.header-tabs {
  display: flex;
  flex-direction: row;
  align-items: stretch;
  flex-basis: auto;
  flex-grow: 0;
  min-width: 0;
}

.header-button-container {
  color: #5e6870;
  display: flex;
  align-items: center;
  padding: 5px;
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
  box-shadow: inset 1px 1px 5px 1px #272c30;
}

#sidebar  {
  height: 100%;
  min-width: 220px;
  max-width: 220px;
  background-color: #30363c;

  display: flex;
  align-items: stretch;
  flex-direction: column;
} 

#sidebar-area {
  box-sizing: border-box;
  flex-basis: 0;
  flex: 1;
  height: 100%;
}

</style>