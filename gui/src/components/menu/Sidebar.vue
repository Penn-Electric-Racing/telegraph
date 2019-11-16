<template>
  <div class="sidebar">
      <transition name="slide">
        <div v-if="isPanelOpen"
           class="sidebar-panel">
            <slot></slot>
        </div>
      </transition>
  </div>
</template>
<script>
import { store, mutations } from '@/store.js'
export default {
  methods: {
    closeSidebarPanel: mutations.toggleNav
  },
  computed: {
    isPanelOpen () {
      return store.isNavOpen
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
.sidebar-panel {
    overflow-y: auto;
    background-color:#2A3F54;
    position: fixed;
    left: 0;
    top: 0;
    height: 100vh;
    z-index: 999;
    padding: 3rem 0px 0rem 0px;
    width: 12%;
}
</style>
