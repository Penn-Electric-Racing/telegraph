import { Signal } from './signal.mjs'

export class Context {

}

export class Namespace {

}

export class Request {
    constructor() {
        this.closed = new Signal();
        this.isClosed = false;
        this.received = new Signal();

        this.buffer = [];
    }
    process(data) {
        if (this.buffer) {
            this.buffer.push(data);
        } else {
            this.received.dispatch(data);
        }
    }
    start() {
        if (this.buffer) {
            var b = this.buffer;
            this.buffer = null;
            for (let m of b) this.received.dispatch(m);
        }
    }
    close() {
        if (!this.isClosed) {
            this.isClosed = true;
            this.closed.dispatch();
        }
    }
}

export class DataQuery {
    constructor(valid=true) {
        this.closed = new Signal();
        this.isClosed = false;
        this.data = [];
        this.received = new Signal();
        this.valid = valid;
    }
    process(data) {
        this.received.dispatch(data);
        this.data.push(...data);
    }
    close() {
        if (!this.isClosed) {
            this.isClosed = true;
            this.closed.dispatch();
        }
    }
}