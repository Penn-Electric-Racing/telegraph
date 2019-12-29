<template>
  <div class="live-view">
    <TreeView class="live-tree" placeholder="No Connection"/>
    <div class="connection-selectors">
      <ComboBox :options="ports"/>
      <ComboBox :options="bauds"/>
      <Button :text="connectionText"/>
    </div>
  </div>
</template>

<script>
import TreeView from './TreeView.vue'

import ComboBox from '../components/ComboBox.vue'
import Button from '../components/Button.vue'

export default {
  name: 'LivePage',
  data: function() {
    return {
      ports : ['/dev/ttyUSB0'],
      bauds : ['Auto', 10, 11, 123],
      connected: false
    }
  },
  computed: {
    connectionText() {
      return this.connected ? 'Disconnect' : 'Connect'
    }
  },
  props: {
    client: Object
  },
  components: {TreeView, ComboBox, Button}
}
</script>

<style scoped>
.live-view {
  height: 100%;
  width: 100%;
  display: flex;
  flex-direction: column;
  align-items: stretch;
}
.live-tree {
  height: 100%;
  width: 100%;
}
.connection-selector {
  width: 100%;
}
</style>
