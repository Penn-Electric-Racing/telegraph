<template>
  <div id="app">
    <LiveConnection>
      <TreeView v-bind:Root = tree></TreeView>
    </LiveConnection>

    <Sidebar>
      <ul class="sidebar-panel-nav">
        <li><button class="button" v-on:click="openLiveConnection"><span>Live Connection</span></button></li>
        <li><button class="button"><span>Logs</span></button></li>
        <li><button class="button"><span>Setting</span></button></li>
      </ul>
    </Sidebar>

    <nav class="main-nav">
      <Burger></Burger>
    </nav>

    <grid id="graph-grid">
      <tile header="Example Tile"/>
    </grid>

  </div>
</template>

<script>
import { mutations } from '@/store.js'
import Tile from './components/Tile.vue'
import Grid from './components/Grid.vue'
import Burger from './components/Menu/Burger.vue'
import Sidebar from './components/Menu/Sidebar.vue'
import LiveConnection from './components/Menu/LiveConnection.vue'
// import DataLog from './components/DataLog.vue'
import { Tree, Group, Variable, Type, Action } from '../../js/lib/tree.mjs'
import TreeView from './components/TreeView.vue'

var main = new Group('Root Tree', 'Main', 'Contains all of the sub-trees', 'some schema', 1)
var acm = new Group('Accumulator Monitoring System', 'AMS', 'Measure variable related to the accumulator', 'testbar/group', 1)
var acmVoltage = new Variable('0x313', 'AMS Voltage', 'Measured in Volts', Type.UINT8)
var testing = new Variable('test', 'tests', 'additional description', Type.BOOL)
var acmCurrent = new Variable('0x311', 'AMS Current', 'Measured in Amps', Type.UINT8)
var speed = new Group('0x311', 'Speed', 'Current Speed of various components', 'g/test', 1)
var frontRpm = new Variable('0x222', 'Front Wheel Speed', 'Measured in RPM', Type.UINT8)
var c = new Action('Non-pretty action', 'Increase Speed', 'Increases RPM of front wheel', Type.BOOL, Type.INT8)

main.addChild(acm)
main.addChild(speed)
acm.addChild(acmVoltage)
acm.addChild(acmCurrent)
main.addChild(testing)
speed.addChild(frontRpm)
speed.addChild(c)

c.setActor((arg) => arg ? 10 : 20)

var t = new Tree(main)

var children = t.getRoot().getChildren()

export default {
  name: 'app',

  data () {
    return {
      children: children,
      tree: t.getRoot()
    }
  },

  computed: {

  },

  components: {
    Tile, Grid, Burger, Sidebar, LiveConnection, TreeView
  },

  methods: {
    openLiveConnection: function (event) {
      mutations.toggleLiveConnection()
    }
  }

}

</script>

<style>
#app {
  font-family: "Poppins", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  background-color:#EDEDED;
  color: #b1b1b5;

  width: 100%;
  height: 100%;
}

#graph-grid {
  position: relative;
  margin-left: 0%;
  margin-right: 0;
  height: auto;
}

html, body {
  border: 0;
  margin: 0;
  width: 100%;
  height: 100%;
}

.logo {
  align-self: center;
  color: #fff;
  font-weight: bold;
  font-family: "Lato";
}

.main-nav {
  display: inline-block;
  justify-content: space-between;
  padding: 0.5rem 0.8rem;
}

h1 {
  font-weight: 30;
}

ul.sidebar-panel-nav {
  list-style-type: none;
  padding-left: 10%;
  margin: 0;
}

ul.liveconnection-panel-nav {
  padding-left: 10%;
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
  border-radius: 4px;
  background-color: #2A3F54;;
  border: none;
  color: #FFFFFF;
  text-align: left;
  font-size: 1.5rem;
  padding: 0px;
  width: auto;
  transition: all 0.5s;
  cursor: pointer;
  padding-bottom: 1.0em;
}

.button span {
  cursor: pointer;
  display: inline-block;
  position: relative;
  transition: 0.5s;
}

.button span:after {
  content: '\00bb';
  position: absolute;
  opacity: 0;
  top: 0;
  right: -20px;
  transition: 0.5s;
}

.button:hover span {
  padding-right: 25px;
}

.button:hover span:after {
  opacity: 1;
  right: 0;
}
</style>
