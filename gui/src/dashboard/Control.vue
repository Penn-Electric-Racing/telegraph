<template>
  <div class="control-display" :class="{horizontal: horizontal}">
    <div class="node-value">{{ stateLabel }}</div>
    <div class="node-label">
      {{ node.getPretty() }}
    </div>
  </div>
</template>

<script>
  import { Node, Action, Variable } from 'telegraph'
  export default {
    name: 'Control',
    props: {
      node: Node,
      horizontal: { default: false, type: Boolean }
    },
    data: function() {
      return {
        sub: null,
        state: null,
      }
    },
    computed: {
      stateLabel() {
        var str = '' + this.state;
        if (typeof this.state == 'number' && this.state % 1 !== 0) {
          str = this.state.toFixed(2);
        }
        return this.state == null ? '...' : '' + str;
      }
    },
    methods: {
      async subscribe() {
        if (this.sub) await this.sub.cancel();
        if (this.node instanceof Variable) {
          this.sub = await this.node.subscribe(0.1, 1);
          if (this.sub) {
            this.sub.data.add(v => this.state = v);
            this.sub.poll();
          } else {
            this.state = 'N/A';
          }
        }
      }
    },
    watch: { node(val) { this.subscribe() } },
    created() { this.subscribe(); },
    destroyed() {
      if (this.sub) this.sub.cancel();
    }
  }
</script>

<style>
  .control-display {
    display: flex;
    flex-direction: column;
    align-items: center;
  }
  .horizontal {
    flex-direction: row-reverse;
    align-items: stretch;
  }

  .node-value {
    font-size: 21px;
    padding: 3px 10px 3px 10px;
    min-width: 3em;
    text-align: center;
  }
  .node-label {
    font-size: 13px;
    color: #fff;
    padding: 1px 10px 5px 10px;
  }
</style>
