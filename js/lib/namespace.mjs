import Signal from 'signals'

export class Query {
  constructor(set) {
    this.all = set;
    this.added = new Signal();
    this.removed = new Signal();
    this.closed = new Signal();
  }

  close() {
    this.closed.dispatch(this);
  }
}

// a special kind of feed specially
// for data queries
export class DataQuery {

}

export class Context {
  // none of these things can change
  // during the lifetime of the context!
  constructor(ns, uuid, name, type, info) {
    this._uuid = uuid;
    this._name = name;

    this._type = type; // 'device', 'archive', or 'container'
    this._info = info; // type specific context information
  }

  getNamespace() { return this._ns; }

  getName() { return this._name; }
  getUUID() { return this._uuid; }
  getType() { return this._type; }
  getInfo() { return this._info; }

  // to be overridden
  async fetch() { return null; }

  // returns a feed object with mounts
  async mounts(srcs=true, tgts=true) { return null; }

  // mount this context on another context
  async mount(src) { return false; }
  async unmount(src) { return false; }

  // context actions, return null or false on failure
  // take a variable/node as the argument
  async subscribe(variable, minInterval, maxInterval) { return null; }
  async call(action, arg) { return null; }
  async writeData(variable, data) { return false; }
  async queryData(variable) { return null; }

  // versions of the above, but path-based
  async subscribePath(path, minInterval, maxInterval) { return null; }
  async callPath(path, arg) { return null; }
  async writeDataPath(path, data) { return false; }
  async queryDataPath(path) { return null; }

  async destroy() { return false; }

  pack() {
    return { ns: this.getNamespace().getUUID(),
           uuid: this.getUUID(),
           name: this.getName(),
           type: this.getType(),
           info: Info.pack(this.getInfo()) }
  }

  toString() {
    return this._name;
  }
}

export class Task {
  constructor(type) {
    this._type = type;
  }
  getType() { return this._type; }
}

// a namespace can handle all of the operations
// defined in api.proto. A relay then exposes a namespace.
export class Namespace {
  constructor(uuid) { this._uuid = uuid; this.destroyed = new Signal() }

  getUUID() { return this._uuid; }

  async createContext(name, type, params, sources) { return null; }
  async destroyContext(name, type, params, sources) { return null; }

  async mounts({srcsOf=null, tgtsOf=null}) {  return null; }
  async contexts({byUuid=null, byName=null, byType=null}) { return null; }
  async tasks({byUuid=null, byName=null, byType=null}) { return null; }

  async fetch(ctxUuid, owner=null) { return null; }

  async subscribe(ctxUuid, path, minInterval, maxInterval) { return null; }
  async call(ctxUuid, path, arg) { return null; }

  async queryData(ctxUuid, path) { return null; }
  async writeData(ctxUuid, path, data) { return null; }

  // returns uuids of created context/task
  async createContext(name, type, info, sources) { return null; }
  async createTask(name, type, info, sources) { return null; }

  async destroyContext(ctxUuid) { return false; }
  async destroyTask(taskUUID) { return false; }

  async startTask(taskUuid) { return false; }
  async stopTask(taskUuid) { return false; }

  async mount(srcUuid, tgtUuid) { return false; }
  async unmount(srcUuid, tgtUuid) { return false; }
}

export var Info = {
  pack(json) {
    var type = typeof json;
    if (Array.isArray(json)) {
      return {isArray: true, array: json.map((x) => Info.pack(x)), object: []};
    } else if (type == "number") {
      return {number:json, array: [], object: []};
    } else if (type == "string") {
      return {str:json, array: [], object: []};
    } else if (type == "object") {
      return { array: [], isObject: true,
               object: Object.entries(json).map(
                 ([k,v]) => { return { key: k, value: Info.pack(v) } } ) };
    }
    return null;
  },
  unpack(protobuf) {
    if (protobuf == null) return null;
    if (protobuf.isArray) {
      return protobuf.array.map(Info.unpack);
    } else if (protobuf.isObject) {
      var obj = {};
      for (let {key, value} of protobuf.object) {
        obj[key] = Info.unpack(value);
      }
      return obj;
    } else {
      return protobuf.number != undefined ? protobuf.number :
             protobuf.str != undefined ? protobuf.str :
             protobuf.object != undefined ? Info.unpack(protobuf.object) : null;
    }
  }
}

