<template>
    <div class="scrollpane">
        <slot></slot>
    </div>
</template>

<script>
export default {
    name: 'ScrollPane'
}
</script>

<style scoped>
.scrollpane {
    overflow-y: auto;
    overflow-x: auto;
    transition: border-color 0.2s ease-in-out;
    border-color: transparent;
}
.scrollpane::-webkit-scrollbar {
    width: 4px;
}
.scrollpane::-webkit-scrollbar-thumb {
    border-right-style: inset;
    border-right-width: calc(100vw + 100vh);
    border-color: inherit;
}
.scrollpane:hover {
    border-color: #1c8ed7;
}
</style>