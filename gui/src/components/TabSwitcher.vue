<template>
  <div class="tabSwitcher">
    <ScrollArea horizontalWheel>
      <div class="tabsList">
        <div :class="{tabHeader:true, active:tab.id==active}"
            v-for="tab in tabs" v-on:click.stop="selectTab(tab.id)" :key="tab.id">
          <span class="tabName" v-if="tab.name != undefined" 
                                @dblclick="requestEdit"
                                @blur="(e) => {requestStopEdit(e, tab.id)}"
                                @keydown.enter.prevent="(e) => {enter(e, tab.id)}">
            {{ tab.name }}
          </span>

          <font-awesome-icon :icon="tab.icon" class="tabIcon" size="sm" v-if="tab.icon != undefined"/>

          <span class="tabClose" v-on:click.stop="closeTab(tab.id)" v-if="closeable">
            <font-awesome-icon icon="times"/>
          </span>
        </div>
      </div>
    </ScrollArea>
  </div>
</template>

<script>
  import TabArea from './TabArea.vue'
  import ScrollArea from './ScrollArea.vue'
  export default {
    name: 'TabSwitcher',
    components: { TabArea, ScrollArea },
    props: {
      tabs: Array, // an array of strings
      active: String,
      closeable: { type: Boolean, default: false },
      editable: { type: Boolean, default: false },
      draggable: { type: Boolean, default: false }
    },
    methods: {
      requestEdit(e) {
        if (this.editable) {
          e.srcElement.contentEditable = true;
        }
      },
      requestStopEdit(e, id) {
        e.srcElement.contentEditable = false;
        this.renameTab(id, e.srcElement.textContent);
      },
      enter(e, id) {
        this.requestStopEdit(e, id);
      },
      selectTab(id) {
        this.$emit('selected', id)
      },
      closeTab(id) {
        this.$emit('closed', id)
      },

      renameTab(id, newName) {
        this.$emit('renamed', id, newName)
      }
    }
  }
</script>

<style scoped>
  .tabsList {
    box-sizing: border-box;
    background-color: #272c30;

    color: #5e6870;
    display: flex;
    flex-direction: row;
    justify-content: left;
    align-items: stretch;
    flex: 1;
  }
  .tabSwitcher {
    display: flex;
    flex-direction: row;
    align-items: stretch;
    min-width: 0px;
  }
  .tabHeader {
    width: 100%;

    color: #5e6870;

    display: flex;
    align-items: center;
    justify-content: center;

    padding-left: 0.3rem;
    padding-right: 0.3rem;

    border-bottom: 2px solid #5e6870;
  }
  .tabClose {
    color: #5e6870;
    margin-left: 5px;
    transition: color 0.5s;
  }
  .tabClose:hover {
    color: #ed4949;
  }

  .tabHeader.active {
    color: #ffffff;
    background-color: #252a2e;
    border-bottom: 2px solid #1c8ed7;
  }

  .tabName {
    display: block;
    padding: 0px;
    width: auto;
    display: inline-block;
    white-space: nowrap;
    min-width: 20px;
    min-height: 1em;
  }

  .tabIcon {
    padding: 7px 5px 7px 5px;
  }
</style>
