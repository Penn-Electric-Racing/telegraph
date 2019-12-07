<template>
    <div class="tile">
        <h1>{{ header }}</h1>
        <slot></slot>
    </div>
</template>

<script>
export default {
  name: 'TileView',
  props: {
    header: String,
  },
  data: function() {
    return {
      x: 0,
      y: 0,
      w: 0,
      h: 0
    }
  }
}
</script>

<style scoped>

.tile {
  position: relative;
  display: flex;
  flex-direction: column;
  word-wrap: break-word;
  background-color: #222437;
  background-clip: border-box;
  border-radius: 0.25rem;
  border: 1px solid #313452
}

</style>
