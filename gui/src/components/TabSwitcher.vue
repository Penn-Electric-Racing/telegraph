<template>
  <div class="tabSwitcher" ref="tabSwitcher">
    <ScrollArea horizontalWheel>
      <div class="tabsList">
        <TabElement v-for="tab in tabs" :tab="tab"
        :closeable="closeable" :editable="editable" :draggable="draggable"
        :active="tab.id==active" @selected="selected" @closed="closed"
        @tabMoved="tabMoved" @renamed="renamed" ref="tabElements">
        </TabElement>
      </div>
    </ScrollArea>
  </div>
</template>

<script>
  import TabArea from './TabArea.vue'
  import ScrollArea from './ScrollArea.vue'
  import interact from 'interactjs'
  import TabElement from './TabElement.vue'

  export default {
    name: 'TabSwitcher',
    components: { TabArea, ScrollArea, TabElement },
    props: {
      tabs: Array, // an array of strings
      active: String,
      closeable: { type: Boolean, default: false },
      editable: { type: Boolean, default: false },
      draggable: { type: Boolean, default: false }
    },
    methods: {
      selected(id) {
        this.$emit('selected', id)
      },
      closed(id) {
        this.$emit('closed', id)
      },
      renamed(id, newName) {
        this.$emit('renamed', id, newName);
      },
      tabMoved(id, x, data_x){
        if (this.$refs.tabSwitcher.getAttribute('switchedCurrentTick') == 'true' && data_x == 0) {
          console.log('switched!');
          this.$refs.tabSwitcher.setAttribute('switchedCurrentTick', false);
          return;
        }
        else {
          console.log(this.$refs.tabSwitcher.getAttribute('switchedCurrentTick'));
        }
        //console.log(x);
        var sizes = this.$refs.tabElements.map( (f) => {
          return f.$el.clientWidth
        });;
        var index;
        for (var i in this.tabs) {
          if (this.tabs[i].id == id) {
            index = i;
            break;
          }
        }
        if (index > 0 && x < -sizes[index - 1]) {
          console.log('switching left');
          this.$refs.tabSwitcher.setAttribute('switchedCurrentTick', true);
          [this.tabs[index - 1], this.tabs[index]] = [this.tabs[index], this.tabs[index - 1]];
          this.$forceUpdate();
          this.$nextTick().then(() => {
            this.$refs.tabElements.forEach(f => {f.resetDrag()});
            //this.$refs.tabElements[index].resetDrag();
            //this.$refs.tabElements[index - 1].resetDrag();
            //this.$refs.tabSwitcher.setAttribute('switchedCurrentTick', false);
          });
        }
        else if (index < sizes.length && x > sizes[index - -1]) {
          console.log('switching right');
          this.$refs.tabSwitcher.setAttribute('switchedCurrentTick', true);
          [this.tabs[index - -1], this.tabs[index]] = [this.tabs[index], this.tabs[index - -1]];
          this.$forceUpdate();
          this.$nextTick().then(() => {
            this.$refs.tabElements.forEach(f => {f.resetDrag()});
            //this.$refs.tabSwitcher.setAttribute('switchedCurrentTick', false);
          });
        }
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
</style>
