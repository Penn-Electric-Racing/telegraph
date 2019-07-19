<template>
    <div class="card">
        <h1>{{ title }}</h1>
        <h2>{{ subtitle }}</h2>
    </div>
</template>

<script>
export default {
  name: 'card',
  props: {
    title: String,
    subtitle: String
  }
}
</script>

<style scoped>

.card {
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
