<template>
  <div class="header">
    <button v-on:click="toggleLiveConnection"><font-awesome-icon icon="wifi" /></button>
    <button v-on:click="toggleSettings"><font-awesome-icon icon="cogs" /></button>
    <button v-on:click="toggleLogs"><font-awesome-icon icon="edit" /></button>
  </div>
</template>

<script>
  export default {
  data () {
    return {
      name: "SidebarHeader"
    }
  },

  methods: {
    toggleLiveConnection: function () {
      this.$emit('toggleLiveConnection')
    },
    toggleSettings: function () {
      this.$emit('toggleSettings')
    },
    toggleLogs: function () {
      this.$emit('toggleLogs')
    }
  }
}
</script>

<style scoped>

button {
  background-color: Transparent;
  border: none;
  color: #5e686f;
  display: block;
  width: 33.3333%;
}

button:active {
  color: white;
}

button:hover{
  background-color:#272c31;
  color: white;
}

.header {
  display: flex;
  flex-direction: row;
  align-items: stretch;
  align-content: stretch;

  width: 100%;
  height: 3rem;
  background-color: #2a3034;
}

</style>
