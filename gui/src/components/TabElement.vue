<template>
  <div :class="{tabHeader:true, active:active}" ref="tabHeader"
      @mousedown="selectTab(tab.id)" :key="tab.id">
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
</template>
<script>
  import TabArea from './TabArea.vue'
  import ScrollArea from './ScrollArea.vue'
  import interact from 'interactjs'
  export default {
    name: 'TabElement',
    components: { TabArea, ScrollArea },
    props: {
      tab: Object,
      closeable: { type: Boolean, default: false },
      editable: { type: Boolean, default: false },
      draggable: { type: Boolean, default: false },
      active: { type: Boolean, default: true }
    },
    mounted() {
      const element = this.$refs.tabHeader
      if (this.draggable) {
        interact(element)
        .draggable({ maunualStart: true})
        .on('dragmove', (event) => {
          var x = parseFloat(event.target.getAttribute('data-x') || 0) + event.dx;
          event.target.setAttribute('data-x', x);
          this.$emit('tabMoved', this.tab.id, x);
        })
        .on('dragend', (event) => {
          event.target.setAttribute('data-x', 0);
        });
      }
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
  .tabHeader {
    width: 100%;
    color: #5e6870;
    display: flex;
    align-items: center;
    justify-content: center;
    padding-left: 0.3rem;
    padding-right: 0.3rem;
    border-bottom: 2px solid #5e6870;
    touch-action: none;
    flex-direction: row;
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
