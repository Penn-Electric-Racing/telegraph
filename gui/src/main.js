import Vue from "vue";
import App from "./App.vue";

import store from "./store";

// import FloatingVue from 'floating-vue';
// import 'floating-vue/dist/style.css';

import { library } from "@fortawesome/fontawesome-svg-core";
import {
	faCogs,
	faWifi,
	faTimes,
	faSlidersH,
	faCubes,
	faBox,
	faChartBar,
	faColumns,
	faEdit,
	faFolderOpen,
	faPlus,
	faTasks,
	faTrash,
	faExchangeAlt,
	faQuestion,
	faServer,
	faMagnet,
	faDatabase,
	faPlay,
	faClock,
	faSatelliteDish,
	faCircle,
	faSquare,
} from "@fortawesome/free-solid-svg-icons";
import { faTrashAlt } from "@fortawesome/free-regular-svg-icons";
import { FontAwesomeIcon } from "@fortawesome/vue-fontawesome";


library.add(
	faTimes,
	faWifi,
	faCogs,
	faEdit,
	faFolderOpen,
	faColumns,
	faBox,
	faPlus,
	faSlidersH,
	faCubes,
	faChartBar,
	faTasks,
	faTrash,
	faTrashAlt,
	faExchangeAlt,
	faQuestion,
	faServer,
	faMagnet,
	faDatabase,
	faSatelliteDish,
	faCircle,
	faSquare,
	faPlay,
	faClock
);

Vue.component("font-awesome-icon", FontAwesomeIcon);

// import VTooltip from "v-tooltip";
// import vuetify from '@/plugins/vuetify'
import FloatingVue from 'floating-vue';

Vue.use(FloatingVue);
import 'floating-vue/dist/style.css';

Vue.use((Vue) => {
	Vue.prototype.$bubble = function $bubble(eventName, ...args) {
		// Emit the event on all parent components
		let component = this;
		do {
			component.$emit(eventName, ...args);
			component = component.$parent;
		} while (component);
	};
});

// Vue.use(VTooltip);


Vue.config.productionTip = false;

new Vue({
	store,
	render: (h) => h(App),
	// vuetify,
}).$mount("#app");
