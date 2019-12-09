<template>
  <div class="tile" v-if="loading">
  </div>
  <div class="tile" v-else>
    <div class="tile-header">
      {{name}}
    </div>
    <div class="tile-content">
      Testing {{id}}
    </div>
  </div>
</template>

<script>
  export default {
    name: 'Tile',
    props: {
      storeLocation: String,
      id: Number,
    },
    data: function() {
      return {
        loading: true,
        name: 'Unknown',
        type: 'Unknown'
      }
    },
    created() {
      setTimeout(() => {
        this.loading = false
      }, 2000);
    }
  }
</script>

<style scoped>
  .tile {
    background-color: #30363c;
    color: #88939C;
    height: 100%;

    border: 0px;
    border-radius: 4px;
  }

  .tile-header {
    padding-top: 11px;
    padding-right: 20px;
    padding-bottom: 9px;
    padding-left: 20px;
    overflow: hidden;
    white-space: nowrap;
    text-overflow: ellipsis;

    border-bottom: 2px solid #1c8ed7;
    background-color: rgba(36,41,44,0.5);
    color: #ffffff;

    font-size: 12px;
    line-height: 16px;
    font-weight: normal;
    text-transform: uppercase;
  }
  .tile-content {
    padding-top: 11px;
    padding-right: 20px;
    padding-bottom: 9px;
    padding-left: 20px;
  }
</style>
