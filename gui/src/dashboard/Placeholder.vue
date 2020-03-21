<template>
  <div class="placeholder" ref="placeholder">
    <div class="placeholder-option" v-for="opt in options"
        :style="{height: optionSize + 'px', width: optionSize + 'px'}">
      <div class="icon-container noDrag" @click="select(opt.type)">
        <font-awesome-icon :icon="opt.icon" :style="{width: '100%', height: '100%'}"/>
      </div>
    </div>
  </div>
</template>

<script>
  export default {
    name: 'Placeholder',
    props: {
      id: String,
      data: Object
    },
    data() { 
      return {
        width: null,
        height: null,
        // TODO: Make a registry
        options: [{type: 'ControlPanel', icon: 'sliders-h'}, 
                  {type: 'Graph', icon: 'chart-bar'}]
      };
    },
    computed: {
      noOptions() {
        return this.options.length == 0;
      },
      optionSize() {
        var aspect = this.width / this.height;
        var k = Math.round(Math.sqrt(this.options.length / aspect));
        var c = Math.ceil(this.options.length / k);
        return Math.min(this.height / k, this.width / c);
      }
    },
    methods: {
      select(type) {
        this.data.type = type;
      }
    },
    mounted() {
      new ResizeObserver(() => {
        if (this.$refs['placeholder']) this.width = this.$refs['placeholder'].offsetWidth;
        if (this.$refs['placeholder']) this.height = this.$refs['placeholder'].offsetHeight;
      }).observe(this.$refs['placeholder']);
    }
  }
</script>

<style>
.placeholder {
  display: flex;
  flex-wrap: wrap-reverse;
  justify-content: center;
  align-items: center;
  height: 100%;
}
.icon-container {
  margin: 10px;
  border: 1px dashed #5e6870;
  color: #5e6870;
  border-radius: 5px;
  padding: 10px;
  transition: color 0.5s, border-color 0.5s;
  cursor: pointer;
}
.icon-container:hover {
  border: 1px dashed #fff;
  color: #fff;
}

</style>
