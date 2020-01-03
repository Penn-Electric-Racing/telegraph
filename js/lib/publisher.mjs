import HighResTimeout from 'high-res-timeout';
import Signal from 'signal';

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
        this._minTimer.restart()
      if (this.maxInterval > 0)
        this._maxTimer.restart()
    } else {
      this._buffered = val;
    }
  }

  change(minInterval, maxInterval) {
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

  cancel() {
    this._publisher._subs.remove(this);
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

  subscribe(cb, minInterval, maxInterval) {
    return new Subscriber(this, minInterval, maxInterval);
  }
}

// an adapter takes in a stream of updates
// and only does min-time fitlering, not
// republishing on max time
export class Adapter {
  // these may be asynchronous functions!
  constructor(changeSubscription, stopSubscription) {
    this._changeSubscription = changeSubscription;
    this._stopSubscription = stopSubscription;
    this._subs = new Set();
  }

  update(val) {
  }

  subscribe(minInterval, maxInterval) {
    var s = {
    };
    this._subs.add(s);
  }
}
