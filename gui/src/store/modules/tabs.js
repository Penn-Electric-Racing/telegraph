import Vue from "vue";
import Vuex from "vuex";

// initial state
const state = () => ({
	testing: 10,
	tabGroups: [
		{
			index: 0,
			width: 50,
			height: 100,
			top: 0,
			left: 0,
			tabs: [],
			activeTab: "", // the active tab Id
		},
		{
			index: 1,
			width: 50,
			height: 50,
			top: 0,
			left: 50,
			tabs: [],
			activeTab: "", // the active tab Id
		},
		{
			index: 2,
			width: 50,
			height: 50,
			top: 50,
			left: 50,
			tabs: [],
			activeTab: "", // the active tab Id
		}
	],
});

// getters
// used to access the state
const getters = {
	getTesting: (state, getters) => {
		return state.testing;
	},
	getTabGroups: (state, getters) => {
		return state.tabGroups;
	},
	getTabs: (state, groupIndex, getters) => {
		return state.tabGroups[groupIndex].tabs;
	},
	getActiveTab: (state, groupIndex, getters) => {
		return state.tabGroups[groupIndex].activeTab;
	},
};

// actions
// used to update the state
const actions = {
	editTesting({ state, commit }, newValue) {
		commit("pushNewValue", newValue);
	},
	editTabGroups({ state, commit }, newTabGroups) {
		commit("pushNewTabGroups", newTabGroups);
	},
	// payload = {groupIndex: ..., newTabs: [...]}
	editTabs({ state, commit }, payload) {
		commit("pushNewTabs", payload);
	},
	// payload = {groupIndex: ..., newActive: "..."}
	editActiveTab({ state, commit }, payload) {
		commit("pushNewActiveTab", payload);
	},
};

// mutations
// used to commit and track state changes
const mutations = {
	pushNewValue(state, newValue) {
		state.testing = newValue;
	},
	pushNewTabGroups(state, newTabGroups) {
		state.tabGroups = newTabGroups;
	},
	// payload = {groupIndex: ..., newTabs: [...]}
	pushNewTabs(state, payload) {
		state.tabGroups[payload.groupIndex].tabs = payload.newTabs;
	},
	// payload = {groupIndex: ..., newActive: "..."}
	pushNewActiveTab(state, payload) {
		state.tabGroups[payload.groupIndex].activeTab = payload.newActive;
	},
};

export default {
	namespaced: true,
	state,
	getters,
	actions,
	mutations,
};
