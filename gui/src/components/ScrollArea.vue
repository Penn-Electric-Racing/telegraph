<template>
    <div class="scrollpane">
        <slot></slot>
    </div>
</template>

<script>
export default {
    name: 'ScrollArea',
    props: {
        horizontalWheel: { type: Boolean, default: false }
    }
}
</script>

<style scoped>
.scrollpane {
    overflow-y: auto;
    overflow-x: auto;
    min-width: 0px;
    min-height: 0px;
    transition: border-color 0.2s ease-in-out;
    border-color: transparent;

    display: flex;
    flex-direction: column;
    align-items: stretch;
    flex: 1;
}
.scrollpane::-webkit-scrollbar {
    width: 3px;
    height: 3px;
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