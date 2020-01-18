import Vue from 'vue'
import App from './App.vue'

import { library } from '@fortawesome/fontawesome-svg-core'
import { faCogs, faWifi, faTimes, faSlidersH, 
         faCubes, faBox, faChartBar, faColumns,
         faEdit, faFolderOpen, faPlus } from '@fortawesome/free-solid-svg-icons'
import { FontAwesomeIcon } from '@fortawesome/vue-fontawesome'

library.add(faTimes, faWifi, faCogs, faEdit, faFolderOpen, faColumns,
            faBox, faPlus, faSlidersH, faCubes, faChartBar)

Vue.component('font-awesome-icon', FontAwesomeIcon)

Vue.config.productionTip = false

new Vue({
  render: h => h(App)
}).$mount('#app')
