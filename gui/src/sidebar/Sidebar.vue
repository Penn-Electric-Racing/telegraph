<template>
  <div class="sidebar-panel">
      <SidebarHeader v-on:toggleLiveConnection="toggleLiveConnection"
                     v-on:toggleSettings="toggleSettings"
                     v-on:toggleLogs="toggleLogs"> </SidebarHeader>
        <!-- <ul class="sidebar-panel-nav"> -->
          <!-- <li><button class="button" v-on:click="openLiveConnection"><span>Live Connection</span></button></li>
          <li><button class="button" v-on:click="openLogs"><span>Logs</span></button></li>
          <li><button class="button" v-on:click="openSettings"><span>Settings</span></button></li> -->
        <!-- </ul> -->
      <div class="page-container">
        <SidebarPage v-bind:isPageOpen = liveConnectionOpened>
          <TreeView v-bind:node = root></TreeView>
        </SidebarPage>
        <SidebarPage v-bind:isPageOpen = logsOpened></SidebarPage>
        <SidebarPage v-bind:isPageOpen = settingsOpened></SidebarPage>
      </div>
  </div>

</template>

<script>
import SidebarHeader from './SidebarHeader.vue'
import SidebarPage from './SidebarPage.vue'
// import Panel from './Panel.vue'
import TreeView from './TreeView.vue'
import { Tree, Group, Variable, Type, Action } from '../../../js/lib/tree.mjs'

var main = new Group('GROUP 1: Root Tree', 'Main', 'Contains all of the sub-trees', 'some schema', 1)
var acm = new Group('GROUP 2: Accumulator Monitoring System', 'AMS', 'Measure variable related to the accumulator', 'testbar/group', 1)
var acmVoltage = new Variable('0x313', 'AMS Voltage', 'Measured in Volts', Type.UINT8)
var testing = new Variable('VARIABLE', 'tests', 'additional description', Type.BOOL)
var acmCurrent = new Variable('0x311', 'AMS Current', 'Measured in Amps', Type.UINT8)
var speed = new Group('GROUP 3: Speed', 'Speed', 'Current Speed of various components', 'g/test', 1)
var frontRpm = new Variable('0x222', 'Front Wheel Speed', 'Measured in RPM', Type.UINT8)
var c = new Action('Non-pretty action', 'Increase Speed', 'Increases RPM of front wheel', Type.BOOL, Type.INT8)
var testing2 = new Variable('VARIABLE', 'tests', 'additional description', Type.BOOL)

main.addChild(acm)
main.addChild(speed)
acm.addChild(acmVoltage)
acm.addChild(acmCurrent)
acm.addChild(testing2)
main.addChild(testing)
speed.addChild(frontRpm)
speed.addChild(c)

c.setActor((arg) => arg ? 10 : 20)

var t = new Tree(main)

export default {
  data () {
    return {
      root: t.getRoot(),
      liveConnectionOpened: false,
      logsOpened: false,
      settingsOpened: false
    }
  },

  methods: {
    toggleLiveConnection: function () {
      this.liveConnectionOpened = true
      this.logsOpened = false
      this.settingsOpened = false
    },

    toggleLogs: function () {
      this.liveConnectionOpened = false
      this.logsOpened = true
      this.settingsOpened = false
    },

    toggleSettings: function () {
      this.liveConnectionOpened = false
      this.logsOpened = false
      this.settingsOpened = true
    }
  },

  props: {
    // sidebarShowing: Boolean
  },

  components: {
    SidebarHeader,
    TreeView,
    SidebarPage
  }
}
</script>
<style>
ul.sidebar-panel-nav {
  list-style-type: none;
  padding-left: 10%;
  margin: 0;
}

ul.liveconnection-panel-nav {
  margin: 0;
  display: inline-block;
  width: auto;
}

ul.liveconnection-panel-nav > li {
  list-style-type: none;
  padding-bottom: 1rem;
  color: #fff;
  text-decoration: none;
  font-size: 0.5rem;
  text-align: center;
}

ul.sidebar-panel-nav > li > .button {
  display: inline-block;
  background-color: Transparent;
  border-radius: 4px;
  border: none;
  color: #FFFFFF;
  text-align: left;
  font-size: 1.2rem;
  padding: 0px 5rem 0px 0px;
  width: auto;
  transition: all 0.5s;
  cursor: pointer;
  padding-bottom: 1.0em;
}

.sidebar-panel {
  overflow-y: auto;
  background-color: #2f363c;
  width: 15%;
  display: flex;
  flex-direction: column;
}

.page-container {
  background-color: #30363c;
  height: 100%;
  width: 100%;
}

</style>
