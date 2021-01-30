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
        .draggable(true)
        .on('dragmove', this.dragMove)
        .on('dragend', this.dragEnd);
        element.setAttribute('starting', false);
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
      },
      resetDrag() {
        this.$refs.tabHeader.setAttribute('starting', true);
        interact(this.$refs.tabHeader).draggable(true)
        .on('dragmove', this.dragMove)
        .on('dragend', this.dragEnd);
        this.$refs.tabHeader.setAttribute('starting', false);
        //console.log('tab' + this.tab.name + ' data-x' + this.$refs.tabHeader.getAttribute('data-x'));
      },
      dragMove(event) {
        if(event.target.getAttribute('starting-id') == null ||
          event.target.getAttribute('starting-id') == "null") event.target.setAttribute('starting-id', this.tab.id);
        var startingId = event.target.getAttribute('starting-id');
        var data_x = (parseFloat(event.target.getAttribute('data-x')) || 0);
        var x =  data_x + event.dx;
        //console.log('event dx: ' + event.dx);
        event.target.setAttribute('data-x', x);
        //console.log('data-x later ' + event.target.getAttribute('data-x'));
        this.$emit('tabMoved', startingId, x, event.target);
      },
      dragEnd(event) {
        if (this.$refs.tabHeader.getAttribute('starting') == false ||
          !this.$refs.tabHeader.getAttribute('starting')) {
          // console.log('not running');
          event.target.setAttribute('starting-id', null);
          this.$emit('resetDrag');
        }
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
