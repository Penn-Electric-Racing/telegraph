import Vue from "vue";
import Vuex from "vuex";
import tabs from "./modules/tabs";
import theme from "./modules/theme";

Vue.use(Vuex);

const debug = true;

export default new Vuex.Store({
	modules: {
		tabs,
		theme,
	},
	debug,
});
