import Vue from "vue";
import Vuex from "vuex";

// initial state
/*
const state = () => ({
	name: "live",
	type: "dummy_device",
	params: "",
});
*/

const state = () => ({
	name: "live",
	type: "dummy_device",
	params: "",
});

// getters
// used to access the state
const getters = {
	getName: (state, getters) => {
		return state.name;
	},
	getType: (state, getters) => {
		return state.type;
	},
	getParameters: (state, getters) => {
		return state.params;
	},
};

// actions
// used to update the state
const actions = {
	editName({ state, commit }, newName) {
		commit("pushNewName", newName);
	},
	editType({ state, commit }, newType) {
		commit("pushNewType", newType);
	},
	editParams({ state, commit }, newParams) {
		commit("pushNewParams", newParams);
	},
};

// mutations
// used to commit and track state changes
const mutations = {
	pushNewName(state, newName) {
		state.name = newName;
	},
	pushNewType(state, newType) {
		state.type = newType;
	},
	pushNewParams(state, newParams) {
		state.params = newParams;
	},
};

export default {
	namespaced: true,
	state,
	getters,
	actions,
	mutations,
};
