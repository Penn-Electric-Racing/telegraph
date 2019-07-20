<template>
  <div id="app">
    <grid id="graph-grid">
      <tile header="Example Tile"/>
    </grid>
  </div>
</template>

<script>
import Tile from './components/Tile.vue'
import Grid from './components/Grid.vue'

export default {
  name: 'app',
  components: {
    Tile, Grid
  }
}
</script>

<style>
#app {
  font-family: "Poppins", sans-serif, "Noto Color Emoji";
  -webkit-font-smoothing: antialiased;
  -moz-osx-font-smoothing: grayscale;

  background-color: #1c1e2f;
  color: #b1b1b5;

  width: 100%;
  height: 100%;
}

#graph-grid {
  width: 100%;
  height: 100%;
}

html, body {
  border: 0;
  margin: 0;
  width: 100%;
  height: 100%;
}

h1 {
  font-weight: 30;
}

</style>
