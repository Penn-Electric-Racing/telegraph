import Vue from 'vue'

export const store = Vue.observable({
  isNavOpen: false,
  isLiveConnectionOpen: false
})

export const mutations = {
  setIsNavOpen (bool) {
    store.isNavOpen = bool
  },
  setIsLiveConnectionOpen (bool) {
    store.isLiveConnectionOpen = bool
  },
  toggleNav () {
    store.isNavOpen = !store.isNavOpen
    if (store.isNavOpen) {
      document.getElementById('graph-grid').style.marginLeft = '12%'
    } else {
      document.getElementById('graph-grid').style.marginLeft = '0%'
      store.isLiveConnectionOpen = false
    }
  },
  toggleLiveConnection () {
    store.isLiveConnectionOpen = !store.isLiveConnectionOpen
    if (store.isLiveConnectionOpen) {
      document.getElementById('graph-grid').style.marginLeft = '24%'
    } else {
      document.getElementById('graph-grid').style.marginLeft = '12%'
    }
  }
}
