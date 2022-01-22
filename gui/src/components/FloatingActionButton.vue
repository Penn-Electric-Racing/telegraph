<template>
  <fab :actions="fabActions"
       @cache="cache"
       @alertMe="alert"
  ></fab>
</template>

<script>
import fab from 'vue-fab'

export default {
  components: {
    fab
  },
  data(){
    return {
      bgColor: '#778899',
      position: 'top-right',
      fabActions: [
        {
          name: 'cache',
          icon: 'cached'
        },
        {
          name: 'alertMe',
          icon: 'add_alert'
        }
      ]
    }
  },
  methods:{
    fun1(){
      //console.log('Cache Cleared');
    },
    fun2(){
      //alert('Clicked on alert icon');
    }
  }
}
</script>

<style scoped>

</style>