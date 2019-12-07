<template>
  <div class="tabview">
    <div class="headers" v-bind:style="{paddingLeft: headersLeftPad}">
      <div v-bind:class="{header:true, active:index==activeIndex}" 
           v-for="(head, index) in tabHeaders" v-on:click="selectTab(index)" :key="index">
        {{ head }}
      </div>
    </div>
    <div class="tabs">
      <slot></slot>
    </div>
  </div>
</template>

<style scoped>
  /* Style from http://demo.adminbootstrap.com/right/1.0.0/index.html */
  .tabview {
    width: 100%;
    height: 100%;
    display: flex;
    flex-direction: column;
  }
  .headers {
    background-color: rgba(36, 41, 44, 0.5);
    box-sizing: border-box;
    height: 50px;
    transition: padding-left 0.3s;

    color: #5e6870;
    display: flex;
    justify-content: left;
  }
  .header {
    box-sizing: border-box;
    height: 50px;

    display: flex;
    align-items: center;
    justify-content: center;

    padding-left: 10px;
    padding-right: 10px;
  }
  .header.active {
    border-bottom: 2px solid #1c8ed7;
    color: #ffffff
  }
</style>

<script>
  export default {
    name: 'TabbedView',
    data: function() {
      return {
        tabHeaders: [],
        activeIndex: -1
      }
    },
    props: {
      headersLeftPad: {
        type: String,
        default: '0px'
      }
    },
    methods: {
      updateHeaders() {
        var headers = []
        var active = -1;
        for (let i = 0; i < this.$children.length;i++) {
          let c = this.$children[i];
          headers.push(c.name)
          if (c.active) {
            active = i;
          }
        }
        this.tabHeaders = headers
        this.activeIndex = active;
      },
      selectTab(index) {
        for (let i = 0; i < this.$children.length;i++) {
          this.$children[i].active = false;
        }
        this.$children[index].active = true;
        this.activeIndex = index;
      }
    },
    mounted() {
      this.updateHeaders()
    },
    unmounted() {
    }
  }
  
</script>
