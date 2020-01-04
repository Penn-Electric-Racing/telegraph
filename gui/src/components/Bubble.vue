<template>
  <div class="bubble" ref="bubble">
    <div class="bubble-header">
      {{ header }}
    </div>
    <div class="bubble-content" v-if="hasContent">
      <slot name="content">
      </slot>
    </div>
  </div>
</template>

<script>
  import interact from 'interactjs'
  export default {
    name: 'Bubble',
    props: {
      header: String,
      draggable: { type: Boolean, default: false },
      hasContent: { type: Boolean, default: false },
      dragData: { type: Object, default: null }
    },
    mounted() {
      if (this.draggable) {
        interact(this.$refs['bubble'])
          .draggable({ manualStart: true, allowFrom: '.bubbleHeader' })
          .on('down', (event) => {
            var interaction = event.interaction;

            // if the pointer was moved while being held down
            // and an interaction hasn't started yet
            if (interaction.pointerIsDown && !interaction.interacting()) {
              var original = event.currentTarget;
                // create a clone of the currentTarget element
              var clone = original.cloneNode(true);
              clone.classList.add('bubbleClone');

              var r = original.getBoundingClientRect();

              clone.style.position = 'absolute';
              clone.style.width = r.width + "px";
              clone.style.height = r.height + "px";
              clone.style.opacity = 0.8;
              clone.style.zIndex = 1000;

              // set the data
              clone.setAttribute('data-x', r.left);
              clone.setAttribute('data-y', r.top);
              clone.style.left = r.left + "px";
              clone.style.top = r.top + "px";

              event.currentTarget.parentNode.parentNode.appendChild(clone);
              // start a drag interaction targeting the clone
              var interacting = interaction.start({ name: 'drag' },
                event.interactable, clone);
              interaction.data = this.dragData;
            }
          }).on('dragmove', function(event) {
            var x = parseFloat(event.target.getAttribute('data-x')) + event.dx;
            var y = parseFloat(event.target.getAttribute('data-y')) + event.dy;
            event.target.setAttribute('data-x', x);
            event.target.setAttribute('data-y', y);
            event.target.style.left = x + 'px';
            event.target.style.top = y + 'px';
          }).on('dragend', function(event) {
            event.target.remove();
          });
      }
    }
  }

    //background-color: #343C42;
</script>

<style scoped>
  .bubble {
    box-shadow: 0px 0px 10px #272c30;
    border-radius: 16px;
    overflow: hidden;
    touch-action: none;
    margin: 10px 2px 10px 2px;
  }
  .bubble-header {
    background-color: #272c30;
    padding: 2px 0px 2px 16px;
  }
  .bubble-content {
    padding: 3px;
    padding-left: 10px;
    background-color: #343C42;
    border-radius: 0px 0px 16px 16px;
  }
</style>