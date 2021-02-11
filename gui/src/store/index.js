import Vue from "vue";
import Vuex from "vuex";
import tabs from "./modules/tabs";

Vue.use(Vuex);

const debug = true;

export default new Vuex.Store({
	modules: {
		tabs,
	},
	debug,
});
