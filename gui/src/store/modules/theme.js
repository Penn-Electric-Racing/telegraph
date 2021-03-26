import Vue from "vue";
import Vuex from "vuex";

// initial state
const state = () => ({
	currentTheme: "dark",
});

// getters
// used to access the state
const getters = {
	getTheme: (state, getters) => {
		return state.currentTheme;
	},
};

// actions
// used to update the state
const actions = {
	editTheme({ state, commit }, newTheme) {
		commit("pushNewTheme", newTheme);
	},
	swapTheme({ state, commit }) {
		if (state.currentTheme == "dark") {
			commit("pushNewTheme", "light");
		} else {
			commit("pushNewTheme", "dark");
		}
	},
};

// mutations
// used to commit and track state changes
const mutations = {
	pushNewTheme(state, newTheme) {
		state.currentTheme = newTheme;
	},
};

export default {
	namespaced: true,
	state,
	getters,
	actions,
	mutations,
};
