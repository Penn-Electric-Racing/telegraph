<template>
  <button class="button" v-on="$listeners">
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
    transition: background-color 0.15s ease-in-out;
    display: inline-block;
    border: none;
    color: white;
    background-color: #1c4ed7;
    font-size: 0.9rem;
    padding: 4px 8px 4px 8px;
    border-radius: 5px;
    min-height: calc(1rem + 12px);
  }
  .button:hover {
    background-color: #265beb;
  }
  .button:focus {
    outline: 0;
  }
  .button:active {
    background-color: #1843b9;
  }
</style>
