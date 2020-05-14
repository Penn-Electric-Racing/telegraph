<template>
  <input class="text-field" :placeholder="placeholder" :value="value" 
          @input="updateValue($event.target.value)"
          @keyup.enter="submit"/>
</template>

<script>
  export default {
    name: 'TextField',
    props: {
      placeholder : { type: String, default: 'Type Text...' },
      value: String
    },
    methods: {
      updateValue(val) {
        this.$emit('input', val);
      },
      submit() {
        this.$emit('submit');
      }
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
    box-shadow: inset 1px 1px 2px 2px #272c30;
    font-size: 1rem;
  }
  .text-field:focus {
    color: #fff;
    outline: 0;
  }
</style>
