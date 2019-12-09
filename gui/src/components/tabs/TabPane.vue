<template>
  <div :class="{tab: true, hidden:!active}" v-if="!backgroundUnload || active">
    <slot/>
  </div>
</template>

<script>
  export default {
    name: 'TabPane',
    props: {
      backgroundUnload: Boolean,
      active: Boolean
    }
  }
</script>

<style scoped>
  .tab {
    width: 100%;
    height: 100%;
  }
  .tab.hidden {
    display: none;
  }
</style>
