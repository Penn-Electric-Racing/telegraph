<template>
  <div class="liveConnection">
      <transition name="slide">
        <div v-if="isPanelOpen"
           class="liveConnection-panel">
            <slot></slot>
        </div>
      </transition>
  </div>
</template>
<script>

export default {
  props: {
    isPanelOpen: Boolean
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
    top: 0;
    height: 100%;
    padding: 3rem 0px 0rem 0px;
    height: 100vh;
}
</style>
