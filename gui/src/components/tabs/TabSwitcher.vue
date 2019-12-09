<template>
  <div class="tabSwitcher">
    <div :class="{tabHeader:true, active:tab.id==activeId}"
         v-for="tab in tabs" v-on:click.stop="selectTab(tab.id)" :key="tab.id">
      {{ tab.name }}
      <span class="tabClose" v-on:click.stop="closeTab(tab.id)">
        <font-awesome-icon icon="times"/>
      </span>
    </div>
    <div class="plus">
    </div>
  </div>
</template>

<script>
  import TabArea from './TabArea.vue'
  export default {
    name: 'TabSwitcher',
    props: {
      tabs: Array, // an array of strings
      activeId: Number
    },
    methods: {
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
  .tabSwitcher {
    box-sizing: border-box;

    color: #5e6870;
    display: flex;
    justify-content: left;
  }
  .tabHeader {
    background-color: #272c30;

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
</style>
