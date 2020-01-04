import hrt from 'high-res-timeout';
var HighResTimeout = hrt.default;
import Signal from 'signals'

class Subscriber {
  constructor(publisher, minInterval, maxInterval) {
    this.data = new Signal();
    this.minInterval = minInterval;
    this.maxInterval = maxInterval;

    this._publisher = publisher;

    this._minTimer = new HighResTimeout(minInterval);
    this._minTimer.on('complete', () => {
      if (this._buffered != undefined) this._notify(this._buffered); 
    });

    this._maxTimer = new HighResTimeout(maxInterval);
    this._maxTimer.on('complete', () => {
      if (this._publisher._lastVal != undefined) 
        this._notify(this._publisher._lastVal);
    });

    this._buffered = undefined;
  }

  _notify(val) {
    if (!this._minTimer.running) {
      this.data.dispatch(val);
      this._buffered = undefined; // no buffered value

      if (this.minInterval > 0) 
        this._minTimer.reset().start();
      if (this.maxInterval > 0)
        this._maxTimer.reset().start();
    } else {
      this._buffered = val;
    }
  }

  async change(minInterval, maxInterval) {
    this.minInterval = minInterval;
    this.maxInterval = maxInterval;

    this._minTimer.duration = minInterval;
    if (this.maxInterval > 0) {
      this._maxTimer.duration = maxInterval;
      this._maxTimer.start(); // make sure started
    } else {
      // make sure stopped if maxInterval is changed to 0
      this._maxTimer.stop(); 
    }
  }

  async cancel() {
    this._minTimer.stop();
    this._maxTimer.stop();
    this._publisher._subs.delete(this);
    this._publisher = null;
  }
}

export class Publisher {
  // is source determines whether
  // to set timers for the max interval
  constructor() {
    this._subs = new Set();
    this._lastVal = undefined;
  }

  update(val) {
    this._lastVal = val;
    for (let s of this._subs) {
      s._notify(val);
    }
  }

  async subscribe(minInterval, maxInterval) {
    var s = new Subscriber(this, minInterval, maxInterval);
    this._subs.add(s);
    setTimeout(() => {
      if (this._lastVal != undefined) s._notify(this._lastVal)
    }, 0);
    return s;
  }
}
