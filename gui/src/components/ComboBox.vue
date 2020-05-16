<template>
  <div>
      <div class="combobox-options"> 
        <Button :key="option" v-for="option in options" 
            @click="setBaudRate(option)">
          {{ option }}
        </Button>
      </div>
      <Button class="combobox-button" ref="button" 
        @click="toggleOptions()"
        :text="selected ? '' : selected"/>
  </div>
</template>

<script>
  import Button from './Button.vue'
  export default {
    name: 'ComboBox',
    components: { Button, vSelect },
    data: function() {
      return { 
        selected: undefined, 
        showing: false,
      };
    },
    props: {
      options: Array,
      title: String
    },
    methods: {
      toggleOptions() {
        this.showing = !this.showing;
        this.$refs.button.blur()
      },
      select(input) {
        this.selected = input;
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