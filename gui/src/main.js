import Vue from 'vue'
import App from './App.vue'
import './registerServiceWorker'
import TreeView from './components/menu/TreeView'

Vue.component('TreeView', TreeView)
Vue.config.productionTip = false

new Vue({
  render: h => h(App)
}).$mount('#app')
