<template>
    <Bubble hasContent noMargin>
        <template v-slot:header>
            <div class="items header-items">
                <span> {{ context.name }} </span>
                <div class="header-icons">
                    <span class="live-icon icon" :class="{liveActive: isLive}" 
                            @click.stop="toggleLive" v-if="canSetLive">
                        <font-awesome-icon icon="satellite-dish"/>
                    </span>
                    <span class="destroy icon" @click.stop="destroy">
                        <font-awesome-icon icon="trash"/>
                    </span>
                </div>
            </div>
        </template>
        <template v-slot:content>
            <div class="content-container">
                <div class="items content-items">
                    <span class="content-type">
                        {{ context.type }}
                    </span>
                    <span class="request-icon icon" @click.stop="streamRequest">
                        <font-awesome-icon icon="exchange-alt"/>
                    </span>
                    <span class="info-icon icon" @click.stop="showInfo">
                        <font-awesome-icon icon="question"/>
                    </span>
                    <span class="data-icon icon" 
                        :class="{dataActive: dataShowing}" @click.stop="toggleData">
                        <font-awesome-icon icon="database"/>
                    </span>
                </div>
                <TreeView :treeRoot="tree" v-if="dataShowing"/>
            </div>
        </template>
    </Bubble>
</template>

<script>
import Bubble from '../components/Bubble.vue'
import TreeView from './TreeView.vue'

import ContextInfoPage from '../pages/ContextInfoPage.vue'
import StreamRequestPage from '../pages/StreamRequestPage.vue'

import uuidv4 from 'uuid/v4';
import Vue from 'vue'

Vue.component('ContextInfoPage', ContextInfoPage);
Vue.component('StreamRequestPage', StreamRequestPage);

import { Context } from 'telegraph'
export default {
    name: 'ContextView',
    components : { Bubble, TreeView },
    props: {
        context: Context,
    },
    computed: {
        canSetLive() {
            return !(this.context.type == 'container' && this.context.name == 'live');
        },
        isLive() {
            for (let x of this.mountedOn) {
                if (x.type == 'container' && x.name == 'live') return true;
            }
            return false;
        }
    },
    data() {
        return {
            dataShowing: false,
            mountedOn: [],
            tree: null
        }
    },
    methods: {
        destroy() {
            if (this.context) {
                this.context.destroy();
            }
        },
        showInfo() {
            // create the popup
            var popup = {
                name: 'Context Information',
                type: 'ContextInfoPage',
                props: {context: this.context},
                id: uuidv4(),
                root: null
            };
            this.$bubble('popup', popup);
        },
        streamRequest() {
            var popup = {
                name: "Requests for " + this.context.name,
                type: 'StreamRequestPage',
                props: {resource: this.context},
                id: uuidv4()
            };
            this.$bubble('popup', popup);
        },
        async toggleLive() {
            if (!this.context) return;
            let ns = this.context.ns;

            let live = ns.contexts
                .extract(x => x.type == 'container' && x.name == 'live');
            if (live == null) {
                // create a new context
                live = await ns.createContext('live', 'container', {}, {src: this.context});
            }
            if (!live) return;
            if (!this.isLive) {
                try {
                    await live.mount(this.context);
                } catch (e) {
                    // if mount fails, destroy
                    // the live context, recreate
                    // and remount
                    await live.destroy();
                    live = await ns.createContext('live', 'container', 
                                                {}, {src: this.context});
                    await live.mount(this.context);
                }
            } else {
                await live.unmount(this.context);
                // if there is nobody else mounted
                // on the container, destroy...
                if (ns.mounts.filter(m => m.tgt == live).empty) {
                    await live.destroy();
                }
            }
        },
        toggleData() {
            this.dataShowing = !this.dataShowing;
            if (!this.dataShowing) {
                this.tree = null;
            } else {
                (async() => {
                    this.tree = await this.context.fetch();
                })();
            }
        },
        onMountAdd(m) {
            if (m.src == this.context) {
                this.mountedOn.push(m.tgt);
            }
        },
        onMountRemove(m) {
            if (m.src == this.context) {
                this.mountedOn.splice(this.mountedOn.indexOf(m.tgt), 1);
            }
        }
    },
    created() {
        if (this.context) {
            this.context.ns.mounts.added.add(this.onMountAdd);
            this.context.ns.mounts.removed.add(this.onMountRemove);
            for (let m of this.context.ns.mounts) {
                this.onMountAdd(m);
            }
        }
    },
    destroyed() {
        if (this.context) {
            this.context.ns.mounts.added.remove(this.onMountAdd);
            this.context.ns.mounts.removed.remove(this.onMountRemove);
        }
    },
    watch: {
        context(n, o) {
            this.mountedOn = [];
            if (o) {
                o.ns.mounts.added.remove(this.onMountAdd);
                o.ns.mounts.removed.remove(this.onMountRemove);
            }
            if (n) {
                n.ns.mounts.added.add(this.onMountAdd);
                n.ns.mounts.removed.add(this.onMountRemove);
                for (let m of n.ns.mounts) {
                    this.onMountAdd(m);
                }
            }
        }
    }
}
</script>

<style scoped>
.items {
    display: flex;
    flex-direction: row;
    padding-right: 12px;
}
.content-items {
    justify-content: flex-start;
    padding-right: 10px;
    padding-left: 10px;
    padding-top: 3px;
    padding-bottom: 4px;
}
.header-items {
    justify-content: space-between;
}
.icon {
    color: #5e6870;
    transition: color 0.3s;
}
.live-icon {
    font-size: 0.8rem;
    margin: 2px;
    margin-right: 0.5rem;
}
.live-icon.liveActive {
    color: #13af4f;
}
.live-icon:hover {
    color: #13af4f;
}
.data-icon {
    font-size: 0.8rem;
    margin: 2px;
}
.data-icon.dataActive {
    color: #1c8ed7;
}
.data-icon:hover {
    color: #1c8ed7;
}
.info-icon, .request-icon {
    font-size: 0.8rem;
    margin: 2px;
    margin-right: 0.5rem;
}
.info-icon:hover, .request-icon:hover {
    color: #e5e5e5;
}
.destroy {
    font-size: 0.7rem;
    transition: color 0.3s;
    align-self: center;
}
.destroy:hover {
    color: #ed4949;
}

.content-type {
    color: #1c8ed7;
    width: 100%;
}
</style>