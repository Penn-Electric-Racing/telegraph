<template>
  <div class="liveConnection">
      <transition name="slide">
        <div v-if="isPanelOpen" class="liveConnection-panel">
          <slot></slot>
          <button class= "closeButton" v-on:click="closePanelMethod">⥢</button>
        </div>
      </transition>
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
.slide-enter-active,
.slide-leave-active
{
  transition: transform 0.2s ease;
}
.slide-enter,
.slide-leave-to {
    transform: translateX(-100%);
    transition: all 150ms ease-in 0s
}
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
