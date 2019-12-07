<template>
  <div class="liveConnection">
    <div v-if="isPanelOpen" class="liveConnection-panel">
      <slot></slot>
      <button class= "closeButton" v-on:click="closePanelMethod">⥢</button>
    </div>
  </div>
</template>

<script>

export default {
  props: {
    isPanelOpen: Boolean,
    closePanel: { type: Function }
  },

  methods: {
    closePanelMethod () {
      this.closePanel()
    }
  }
}
</script>
<style>
.liveConnection-panel {
    overflow-y: auto;
    background-color:#4272f5;
    display: flex;
    height: 100%;
    padding: 3rem 3rem 0rem 1rem;
    height: 100vh;
}

.liveConnection-panel > .closeButton {
  position: absolute;
  /* right: 0%; */
  /* top: 0%; */
  /* padding-top: 0.5rem; */
  margin-top: -2.5rem;
  z-index: 100;
  color: white;
  border: none;
  background-color: Transparent;
  outline: none;
  font-size: 1.5rem;
}
</style>
