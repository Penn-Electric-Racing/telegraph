import hrt from 'high-res-timeout';
var HighResTimeout = hrt.default;
import Signal from 'signals';

class AdapterSubscriber {
  constructor(adapter, type, minInterval, maxInterval) {
    this.type = type;
    this.minInterval = minInterval;
    this.maxInterval = maxInterval;
    this.data = new Signal();

    this._adapter = adapter;

    this._minTimer = new HighResTimeout(minInterval);
    this._minTimer.on('complete', () => { 
      if (this._buffered != undefined) this._notify(this._buffered);
    });
    this._buffered = undefined;
  }

  _notify(val) {
    if (!this._minTimer.running) {
      this.data.dispatch(val);
      this._buffered = undefined;
      if (this.minInterval > 0) this._minTimer.reset().start();
    } else {
      this._buffered = val;
    }
  }

  async change(minInterval, maxInterval, timeout) {
    this.minInterval = minInterval;
    this.maxInterval = maxInterval;

    this._minTimer.duration = minInterval;
    await this._adapter._recalculate();
  }

  async cancel(timeout=1) {
    this._minTimer.stop();
    this._adapter._subs.delete(this);
    await this._adapter._recalculate(timeout);
    this._adapter = null;
  }

  poll() {
    this._adapter.poll();
  }
}

// an adapter takes in a stream of updates
// and only does min-time fitlering, not
// republishing on max time
export class Adapter {
  // these may be asynchronous functions!
  constructor(poll, changeSubscription, stopSubscription) {
    this._type = null;
    this._poll = poll;
    this._changeSubscription = changeSubscription;
    this._stopSubscription = stopSubscription;
    this._subs = new Set();

    this._minInterval = null;
    this._maxInterval = null;
  }

  async _recalculate(timeout) {
    var min = null;
    var max = null;
    for (let s of this._subs) {
      min = min == null ? s.minInterval : Math.min(s.minInterval, min);
      max = max == null || max == 0 ? 
          s.maxInterval : Math.min(s.maxInterval, max);
    }
    if (min != this._minInterval || 
       max != this._maxInterval) {
      this._minInterval = min;
      this._maxInterval = max;
      if (this._subs.size == 0) {
        await this._stopSubscription(timeout);
      } else {
        this._type = await this._changeSubscription(min, max, timeout);
      }
    }
  }

  poll() {
    this._poll();
  }

  update(val) {
    for (let s of this._subs) s._notify(val);
  }

  async subscribe(minInterval, maxInterval, timeout) {
    var s = new AdapterSubscriber(this, 
          this._type, minInterval, maxInterval);
    this._subs.add(s);
    try {
      await this._recalculate(timeout);
      s._type = this._type;
    } catch (e) {
      this._subs.delete(s);
      return null;
    }
    return s;
  }

  // closing the adapter
  // will cancel all associated
  // subscriptions
  close() {
  }
}

