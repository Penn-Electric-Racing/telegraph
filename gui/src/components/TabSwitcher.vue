<template>
  <div class="tabSwitcher" ref="tabSwitcher">
    <ScrollArea horizontalWheel>
      <div class="tabsList">
        <TabElement v-for="tab in tabs" :tab="tab"
        :closeable="closeable" :editable="editable" :draggable="draggable"
        :active="tab.id==active" @selected="selected" @closed="closed"
        @tabMoved="tabMoved" @renamed="renamed" @resetDrag="resetDrag" ref="tabElements">
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
      tabMoved(id, x, target) {
        console.log('x: ' + x);
        //if (!this.$refs.tabSwitcher.getAttribute('switchedWidth'))
        //  this.$refs.tabSwitcher.setAttribute('switchedWidth', 0);

        //if (this.$refs.tabSwitcher.getAttribute('switchedWidth') != 0) {
        //  let data_x = parseFloat(target.getAttribute('data-x'));
        //  let switchedWidth = parseFloat(this.$refs.tabSwitcher.getAttribute('switchedWidth'));
        //  console.log('switchedWidth' + switchedWidth);
        //  target.setAttribute('data-x', data_x - switchedWidth);
        //  this.$refs.tabSwitcher.setAttribute('switchedWidth', 0);
        //  return;
        //}
        //console.log(x);
        let tabs = this.$store.tabs;
        let sizes = this.$refs.tabElements.map( (f) => {
          return f.$el.clientWidth
        });
        let index;
        for (let i in this.tabs) {
          if (this.tabs[i].id == id) {
            index = i;
            break;
          }
        }
        for(let i = 0; i < sizes.length ; i++){
          console.log(sizes[i])
        }
        if (index > 0 && x < -sizes[index - 1]) {
          console.log('switching left');
          let data_x = parseFloat(target.getAttribute('data-x'));
          target.setAttribute('data-x', data_x - -sizes[index - 1]);
          this.$refs.tabSwitcher.setAttribute('switchedWidth', -sizes[index - 1]);
          [this.tabs[index - 1], this.tabs[index]] = [this.tabs[index], this.tabs[index - 1]];
          this.$forceUpdate();
        }
        else if (index < sizes.length && x > sizes[index - -1]) {
          console.log('switching right');
          let data_x = parseFloat(target.getAttribute('data-x'));
          target.setAttribute('data-x', data_x - sizes[index - 1]);
          this.$refs.tabSwitcher.setAttribute('switchedWidth', sizes[index - -1]);
          [this.tabs[index - -1], this.tabs[index]] = [this.tabs[index], this.tabs[index - -1]];
          this.$forceUpdate();
        }
      },
      resetDrag() {
        console.log('resetting drag');
        this.$refs.tabElements.forEach(f => {f.resetDrag()});
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
