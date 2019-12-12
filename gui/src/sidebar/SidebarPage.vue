<template>
  <div v-if="isPageOpen" class="page">
    <slot></slot>
  </div>
</template>

<script>

export default {
  props: {
    isPageOpen: Boolean,
  },

}
</script>

<style>
.page {
    overflow-y: auto;
}
</style>