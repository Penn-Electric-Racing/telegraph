<template>
  <button class="button">
    {{ text }}
  </button>
</template>

<script>
  export default {
    name: 'Button',
    props: {
      text: String
    }
  }
</script>

<style scoped>
  .button {
    display: inline-block;
    border: 0;
  }
</style>
