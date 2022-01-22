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
          name: 'fun1',
          icon: 'cached'
        },
        {
          name: 'fun2',
          icon: 'add_alert'
        }
      ]
    }
  },
  methods:{
    fun1(){
      //any arbitrary function
    },
    fun2(){
      //any arbitrary function
    }
  }
}
</script>

<style scoped>

</style>