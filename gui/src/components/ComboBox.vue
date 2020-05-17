<template>
  <div class="combobox" ref="combobox" tabindex="0" @blur="close">
      <div class="combobox-options" v-if="showing"
          :style="{width: boxWidth + 'px'}">
        <div class="option" :key="option" v-for="option in options" 
            @click="selected(option)">
          {{ option }} <br/>
        </div>
      </div>
      <div class="combobox-area" ref="button" 
              @click="toggleOptions()">
        <p class="area-text" v-if="value != null">{{ value }}</p>
        <p class="area-text placeholder" v-else>{{ placeholder }}</p>
      </div>
  </div>
</template>

<script>
  import Button from './Button.vue'
  export default {
    name: 'ComboBox',
    components: { Button },
    data: function() {
      return { 
        showing: false,
      };
    },
    props: {
      options: Array,
      value: [String, Number],
      placeholder: {type:String, default:'Choose...'}
    },
    methods: {
      toggleOptions() {
        this.showing = !this.showing;
      },
      close() {
        this.showing = false;
      },
      selected(input) {
        this.showing = false;
        this.$emit('input', input);
      },
    },
    mounted() {
      new ResizeObserver(() => {
        if (this.$refs['combobox'])
          this.boxWidth = this.$refs['combobox'].offsetWidth;
      }).observe(this.$refs['combobox']);
    },
    changed: {
      options() {
        if (this.value != null && 
            this.options.indexOf(this.value) < 0) {
          this.$emit('input', input);
        }
      }
    }
  }
</script>

<style scoped>
  .combobox {
    display: flex;
    flex-direction: column;
    align-items: stretch;

    font-size: 0.8rem;
    flex: 1;
    user-select: none;
  }
  .combobox:focus {
    outline: none;
  }
  .combobox-area {
    flex: 1;
    display: flex;
    flex-direction: column;
    justify-items: center;

    border-radius: 5px;
    box-shadow: inset 1px 1px 2px 2px #272c30;
    background-color: #2b3136;
    padding: 3px;
  }
  .area-text {
    margin: 0;
    display: flex;
    flex-wrap: wrap-reverse;
    justify-content: center;
    align-items: center;
    height: 100%;
    text-overflow: ellipsis;
  }
  .combobox-options {
    position: absolute;
    transform: translateY(-100%);
    display: flex;
    flex-direction: column;
    align-items: stretch;
    align-content: stretch;
    background-color: #2b3136;
    border-radius: 7px;
    box-shadow: 1px 1px 5px 1px #272c30;
    padding: 0;
  }
  .option {
    text-align: center;
    transition: background-color 0.2s ease;
    padding: 3px;
    border-radius: 7px;
    width: 100%;
  }
  .option:hover {
    background-color: #363e44;
  }
</style>