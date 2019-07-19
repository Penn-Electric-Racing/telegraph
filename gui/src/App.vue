<template>
  <div id="app">
    <card title="Hi" subtitle="Hello"/>
  </div>
</template>

<script>
import card from './components/card.vue'

export default {
  name: 'app',
  components: {
    card
  }
}
</script>

<style>
#app {
  font-family: "Poppins", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;
  text-align: center;
  margin-top: 60px;
}

body {
  background-color: #1c1e2f;
  color: #b1b1b5;
  border: 0;
  margin: 0;
}

h1 {
  font-weight: 30;
}

</style>
