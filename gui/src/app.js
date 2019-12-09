import {Signal} from 'signals'

export class AppStore {
    constructor() {
        this.dashboardAdded = new Signal();
        this.dashboardRemoved = new Signal();
    }

    fetchDashboards() {
        return [];
    }

    // return the dashstore for this location
    dashboard(location) {
        return null;
    }
}

export class SettingsStore {

}

export class DashboardStore {
    constructor() {
    }

    tile(id) {
    }
}

export class TileStore {
}

