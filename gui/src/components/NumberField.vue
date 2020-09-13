<template>
  <input class="text-field" :value="value" 
          @input="updateValue($event.target.value)"
          @keyup.enter="submit" size=3/>
</template>

<script>
  export default {
    name: 'NumberField',
    props: {
      value: { type: Number, default: 0 }
    },
    methods: {
      updateValue(val) {
        let value = Number.parseInt(val);
        if (isNaN(value)) return;
        this.$emit('input', value);
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
