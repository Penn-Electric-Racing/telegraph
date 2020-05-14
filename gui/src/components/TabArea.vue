<template>
  <div class="tabarea">
    <slot></slot>
  </div>
</template>

<style scoped>
  .tabarea {
    flex: 1;
    display: flex;
    flex-direction: column;
  }
  .header {
    box-sizing: border-box;
    height: 50px;
    background-color: #272c30;

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
    name: 'TabArea',
    methods: {
    }
  }
</script>
