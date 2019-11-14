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
import { store, mutations } from '@/store.js'
export default {
  methods: {
    closeSidebarPanel: mutations.toggleLiveConnection
  },
  computed: {
    isPanelOpen () {
      return store.isLiveConnectionOpen
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
    position: fixed;
    left: 12%;
    top: 0;
    height: 100vh;
    z-index: 999;
    padding: 3rem 0px 0rem 0px;
    width: 12%;
}
</style>
