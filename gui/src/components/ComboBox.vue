<template>
  <div>
    <div id="top-container" v-show="showing"> 
      <div class="combobox-options"> 
        <button class="options" :key="option" v-for="option in options" @focus="setBaudRate(option)">
          {{ option }}
        </button>
      </div>
    </div>

    <div class="bottom-container">
      <button class="combobox-button" ref="button" @click="showing ? hideOptions() : showOptions()" v-on:blur="waitToSetBaudRate()" v-text="baudRate"/>
    </div>
  </div>
</template>

<script>
  export default {
    name: 'ComboBox',
    data: function() {
      return { 
        selected: undefined, 
        showing: false,
        baudRate: this.title,
      };
    },
    props: {
      options: Array,
      title: String
    },
    methods: {
      hideOptions() {
        this.showing = false
        this.$refs.button.blur()
      },
      showOptions() {
        this.showing = true
      },
      setBaudRate(input) {
        this.baudRate = input 
      },

      waitToSetBaudRate() {
        this.showing = true
        setTimeout(() => {
          this.showing = false
        })
      }
    },
  }
  
</script>

<style scoped>

  #top-container {
    position: relative;
    background-color: blue;
  }

  .combobox-options {
    position: absolute;
    bottom: 0;
    width: 100%
  }

  .bottom-container{
    position: relative;
    height: 100%;
    width: 100%;
  }

  .options:focus {
    outline: none
  }

  .options {
    appearance: none;
    -webkit-appearance: none;
    -moz-appearance: none;

    width: 100%;
    height: 100%;

    position: relative;

    color: #5e6870;
    background-color: #353c42;
    /* border-color: #272c30; */
    border: none;

    font-size: 1rem;
  }

  .options:hover {
    background-color: rgba(36, 41, 45, 1);
  }

  .combobox-button {
    height: 100%;
    width: 100%;
    font-size: 1rem; 
    /* background-color: aqua; */
    background-color: rgba(36, 41, 44, 0.5);
    /* border-color: rgba(36, 41, 44, 0.5); */
    border: none;
    color: #5e6870;
  }

  .combobox-button:focus { 
    outline:0 !important; 
    background-color: rgba(36, 41, 45, 1);
    /* border-color:  rgba(36, 41, 45, 1); */
  }

</style>
