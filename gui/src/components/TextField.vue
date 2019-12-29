<template>
  <input class="text-field" :placeholder="placeholder"/>
</template>

<script>
  export default {
    name: 'TextField',
    props: {
      placeholder : { type: String, default: 'Type Text...' }
    }
  }
</script>

<style scoped>
  .text-field {
    padding: 4px 3px 2px 4px;
    border: none;
    color: #6f7981;
    background-color: #2B3136;
    border: 1px solid #272c30;
    box-shadow: inset 3px 3px 3px 3px #272c30;
    font-size: 1rem;
  }
  .text-field:focus {
    color: #fff;
    outline: 0;
  }
</style>
