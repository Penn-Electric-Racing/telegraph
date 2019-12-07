<template>
  <div 
    :id="id"
    class="cards"
    :draggable="draggable"
    @dragstart="dragStart"
    @dragover.stop
    >  
    <slot></slot>
  </div>
</template>

<script>
export default {
  props: ['id', 'draggable'],

  methods: {
    dragStart: e => {
      const target = e.target;

      e.dataTransfer.setData('variable_id', target.id);

      // setTimeout(() => {
      //   target.style.display = "none";
      // }, 0);
   }
  }
}
</script>
