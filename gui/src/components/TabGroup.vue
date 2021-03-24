<template>
    <div class="tabgroup" ref="tabgroup" :style="cssVars">
        <div class="header-tabs">
            <div class="header-button-container">
                <FlatButton id="new-tab" icon="plus" @click="newDashboard" />
            </div>
            <TabSwitcher
                :tabs="tabgroup.tabs"
                :active="tabgroup.activeTab"
                :closeable="true"
                :editable="true"
                :draggable="true"
                @selected="selectTab"
                @closed="closeTab"
                @renamed="renameTab"
            />
        </div>
        <div class="tabArea">
            <TabArea>
                <component
                    v-for="tab in tabgroup.tabs"
                    v-show="tab.id == tabgroup.activeTab"

                    :is="tab.type"
                    :name="tab.name"
                    :id="tab.id"
                    :key="tab.id"
                    :nsQuery="nsQuery"

                    v-bind="tab.props"
                />

                <!-- TODO
                    @popup="newPopup"
                    @newtab="newTab"
                    @renamed="
                        (name) => {
                            renameTab(tab.id, name);
                        }
                    "
                    @close="
                        () => {
                            closeTab(tab.id);
                        }
                    " -->
            </TabArea>
        </div>
    </div>
</template>

<style scoped>
.tabgroup {
	flex: 1;
	display: flex;
	flex-direction: column;
    height: 100%;
    border-style: dashed;
    border-color: white;
    position: absolute;
    top: var(--top);
    left: var(--left);
    max-width: var(--width);
    width: var(--width);
}

.tabArea {
    width: 100%;
    height: 100%;
}

.header-tabs {
	display: flex;
	flex-direction: row;
	align-items: stretch;
	flex-basis: auto;
	flex-grow: 0;
	min-width: 0;
}

.header-button-container {
	color: var(--button-text-color);
	display: flex;
	align-items: center;
	padding: 5px;
}
</style>

<script>
import TabSwitcher from "./TabSwitcher.vue";
import FlatButton from "./FlatButton.vue";
import TabArea from "./TabArea.vue";

import Dashboard from "../dashboard/Dashboard.vue";

import uuidv4 from "uuid/v4";
import Vue from "vue";

import { NamespaceQuery } from 'telegraph';

export default {
	name: "TabGroup",
    components: { TabSwitcher, FlatButton, TabArea, Dashboard },
    props: {
		tabgroup: Object,
        nsQuery: NamespaceQuery,
        dashboards: Object,
		closeable: { type: Boolean, default: false },
		editable: { type: Boolean, default: false },
		draggable: { type: Boolean, default: false },
	},
    computed: {
        cssVars() {
            return {
                "--top": this.tabgroup.top + "%",
                "--left": this.tabgroup.left + "%",
                "--width": this.tabgroup.width + "%",
                "--height": this.tabgroup.height + "%",
            }
        }
    },
	methods: {
        newDashboard() {
            var dashData = Vue.observable({
				widgets: {},
				layout: [],
				info: { name: "Untitled" },
			});
			var id = uuidv4();

			this.tabgroup.tabs.push(
				Vue.observable({
					type: "Dashboard",
					name: "Untitled",
					props: { data: dashData },
					id: id,
				})
			);

			if (this.tabgroup.activeTab == "") {
                var payload = {"newActive": id, "groupIndex": this.tabgroup.index};
                this.$store.dispatch("tabs/editActiveTab", payload);
            }
        },
        selectTab(id) {
            var payload = {"newActive": id, "groupIndex": this.tabgroup.index};
            this.$store.dispatch("tabs/editActiveTab", payload);
        },
        closeTab(id) {
			this.tabgroup.tabs.splice(
				this.tabgroup.tabs.findIndex((tab) => tab.id == id),
				1
			);
			if (this.tabgroup.activeTab == id) {
				this.tabgroup.activeTab = null;

				if (this.tabgroup.tabs.length > 0) {
                    this.tabgroup.activeTab = this.tabgroup.tabs[0].id;
                }
			}
        },
        renameTab(id, name) {
            for (let t of this.tabgroup.tabs) {
				if (t.id == id) {
					// TODO: right now we assume all tabs are dashboards
					t.name = name;
					t.props.data.info.name = name;
					if (!this.dashboards[t.id]) {
						this.$set(this.dashboards, t.id, t.props.data);
					}
					break;
				}
			}
        }
    },
};
</script>
