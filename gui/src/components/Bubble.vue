<template>
  <div class="bubble" ref="bubble" v-on="$listeners">
    <div class="bubble-stack">
      <div class="bubble-header" ref="bubble-header">
        <slot name="header">
        </slot>
      </div>
      <div class="bubble-content" 
            :class="{indent}" v-if="hasContent">
        <slot name="content">
        </slot>
      </div>
    </div>
    <div class="bubble-sidebar" v-if="hasSidebar">
        <slot name="sidebar">
        </slot>
    </div>
  </div>
</template>

<script>
  import interact from 'interactjs'
  export default {
    name: 'Bubble',
    props: {
      draggable: { type: Boolean, default: false },
      hasContent: { type: Boolean, default: false },
      indent: { type: Boolean, default: false },
      hasSidebar: { type: Boolean, default: false },
      dragData: { type: Object, default: null },
    },
    mounted() {
      if (this.draggable) {
        interact(this.$refs['bubble-header'])
          .draggable({ manualStart: true })
          .on('down', (event) => {
            var interaction = event.interaction;
            var el = event.srcElement;
            while (el != event.currentTarget) {
              if (el.classList.contains('noBubbleDrag')) {
                return;
              }
              el = el.parentNode;
            }

            // if the pointer was moved while being held down
            // and an interaction hasn't started yet
            if (interaction.pointerIsDown && !interaction.interacting()) {
              var original = event.currentTarget.parentNode.parentNode;
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
    border-radius: 10px;
    overflow: hidden;
    touch-action: none;
    margin: 10px 2px 10px 2px;

    display: flex;
    flex-direction: row;
  }
  .bubble-stack {
    width: 100%;
  }
  .bubble-sidebar {
    width: 32px;
    display: flex;
    flex-direction: column;
  }
  .bubble-header {
    background-color: #272c30;
    padding: 2px 0px 2px 16px;
    display: flex;
    flex-direction: row;
  }
  .bubble-content {
    background-color: #343C42;
    border-radius: 0px 0px 16px 16px;
  }
  .indent {
    padding-left: 10px;
  }
</style>
