import Vue from "vue";
import Vuex from "vuex";

// initial state
const state = () => ({
	testing: 10,
	tabgroup: {
		id: 0,
		tabs: [],
		activeTab: "", // the active tab Id
		children: [
			{
				id: 1,
				tabs: [],
				activeTab: "", // the active tab Id
				children: [],
				layout: "TabGroup"
			}, 
			{
				id: 2,
				tabs: [],
				activeTab: "", // the active tab Id
				children: [],
				layout: "TabGroup"
			},
			{
				id: 3,
				tabs: [],
				activeTab: "", // the active tab Id
				children: [
					{
						id: 4,
						tabs: [],
						activeTab: "", // the active tab Id
						children: [],
						layout: "TabGroup"
					}, 
					{
						id: 5,
						tabs: [],
						activeTab: "", // the active tab Id
						children: [
							{
								id: 6,
								tabs: [],
								activeTab: "", // the active tab Id
								children: [],
								layout: "TabGroup"
							},
							{
								id: 7,
								tabs: [],
								activeTab: "", // the active tab Id
								children: [],
								layout: "TabGroup"
							}
						],
						layout: "HorizontalSplitTabGroup"
					}
				],
				layout: "VerticalSplitTabGroup"
			}
		],
		layout: "HorizontalSplitTabGroup"
	},
});

// getters
// used to access the state
const getters = {
	getTesting: (state, getters) => {
		return state.testing;
	},
	getTabGroup: (state, getters) => {
		return state.tabgroup;
	},

	// TODO fix
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
	editTabGroup({ state, commit }, newTabGroup) {
		commit("pushNewTabGroup", newTabGroup);
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
	pushNewTabGroup(state, newTabGroup) {
		state.tabgroup = newTabGroup;
	},
	// payload = {groupIndex: ..., newTabs: [...]}
	pushNewTabs(state, payload) {
		var tg = findTabGroup(state, payload.groupIndex);

		if (tg) {
			tg.tabs = payload.newTabs;
		} else {
			throw "Tab Group does not exist with id: " + payload.groupIndex;
		}
	},
	// payload = {groupIndex: ..., newActive: "..."}
	pushNewActiveTab(state, payload) {
		var tg = findTabGroup(state, payload.groupIndex);

		if (tg) {
			tg.activeTab = payload.newActive;
		} else {
			throw "Tab Group does not exist with id: " + payload.groupIndex;
		}
	},
};

export default {
	namespaced: true,
	state,
	getters,
	actions,
	mutations,
};

/**
 * Search algos to find specific tabgroup obj
 * 
 * Uses BFS
 */
function findTabGroup(state, tabgroupId) {
	return bfsTabGroup(state.tabgroup, tabgroupId);
}

function bfsTabGroup(root, id) {
	if (!root) {
		return false;
	}

	if (root.id == id) {
		return root;
	}

	for (var i = 0; i < root.children.length; i++) {
		var possibleRetVal = bfsTabGroup(root.children[i], id);

		if (possibleRetVal) {
			return possibleRetVal;
		}
	}

	return false;
}