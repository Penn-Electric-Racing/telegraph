import {AppStore, DashStore, TileStore} from './app.js'

class DummyApp extends AppStore {
    constructor() {
        super()
        this.dashboards = new Map();
    }

    addDashboard(name, location, dashboard) {
        this.dashboards.put(location, 
            {name: name,
             location: location,
             dashboard: dashboard})
    }

    *fetchDashboards() {
        for (let v of this.dashboards.values()) {
            yield [v.name, v.location]
        }
    }
}
