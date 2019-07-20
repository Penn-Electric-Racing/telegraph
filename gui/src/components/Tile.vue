<template>
    <div class="tile">
        <h1>{{ header }}</h1>
    </div>
</template>

<script>
export default {
  name: 'tile',
  props: {
    header: String
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
