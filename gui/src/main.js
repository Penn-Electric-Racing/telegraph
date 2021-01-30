import Vue from "vue";
import App from "./App.vue";

import store from "./store";

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

Vue.config.productionTip = false;

new Vue({
	store,
	render: (h) => h(App),
}).$mount("#app");
