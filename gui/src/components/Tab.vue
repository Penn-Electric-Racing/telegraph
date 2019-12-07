<template>
  <div class="tab" v-if="active">
    <slot/>
  </div>
</template>

<script>
  export default {
    name: 'Tab',
    props: {
      name: String,
    },
    data: function() {
      return { active: false }
    }
  }
</script>
