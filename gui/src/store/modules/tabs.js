import Vue from "vue";
import Vuex from "vuex";

// initial state
const state = () => ({
	tabs: [],
	activeTab: "", // the active tab ID
});

// getters
// used to access the state
const getters = {
	getTabs: (state, getters) => {
		return state.tabs;
	},
	getActiveTab: (state, getters) => {
		return state.activeTab;
	},
};

// actions
// used to update the state
const actions = {
	editTabs({ state, commit }, newTabs) {
		commit("pushNewTabs", newTabs);
	},
	editActiveTab({ state, commit }, newActive) {
		commit("pushNewActiveTab", newActive);
	},
};

// mutations
// used to commit and track state changes
const mutations = {
	pushNewTabs(state, newTabs) {
		state.tabs = newTabs;
	},
	pushNewActiveTab(state, newActive) {
		state.activeTab = newActive;
	},
};

export default {
	namespaced: true,
	state,
	getters,
	actions,
	mutations,
};
