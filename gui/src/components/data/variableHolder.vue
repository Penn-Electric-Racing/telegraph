<template>
  <div
      :id="id"
      class="board"
      @dragover.prevent
      @drop.prevent="drop">
      <slot></slot>
  </div>
</template>

<script>
export default {
  props: {
    id: String
  },

  methods: {
    drop: e => {
      const variable_id = e.dataTransfer.getData('variable_id');

      const variableLabel = document.getElementById(variable_id);

      variableLabel.style.display = "block";
      var clone = variableLabel.cloneNode(true);
      
      console.log("drop");
      console.log(variable_id);
      console.log(this.id);
      // if (this.id == variable_id){
        e.target.appendChild(clone);
      // }
    }
  }
}
</script>