<template>
  <div class="combobox">
    <div class="combobox-button">
      {{ selected }}
    </div>
    <div class="combobox-options" v-show="showing">
      <div class="option" :key="option" v-for="option in options">
        {{ option }}
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    name: 'ComboBox',
    data: function() {
      return { selected: undefined, showing: false };
    },
    props: {
      options: Array
    }
  }
</script>

<style scoped>
  .combobox {
    appearance: none;
    -webkit-appearance: none;
    -moz-appearance: none;

    background-color: inherit;
    text-color: inherit;
  }
  .select-selected {
    background-color: white;
  }
  .select-selected:after {
    position: absolute;
    content: "";
    top: 14px;
    right: 10px;
    width: 0;
    height: 0;
    border: 6px solid transparent;
    border-color: #fff
  }
</style>
