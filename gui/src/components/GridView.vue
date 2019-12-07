<template>
  <div class="grid">
    <slot></slot>
  </div>
</template>

<script>
export default {
  name: 'GridView',
  data: function () {
    return {}
  },
  props: {
    columns: {
      type: Number,
      default: 10
    },
    rows: {
      type: Number,
      default: 10
    }
  }
}
</script>

<style scoped>
.grid {
  width: 100%;
  height: 100%
}
</style>
