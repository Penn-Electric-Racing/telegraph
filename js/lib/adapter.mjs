import hrt from 'high-res-timeout';
var HighResTimeout = hrt.default;
import Signal from 'signals';

class AdapterSubscriber {
  constructor(adapter, type, debounce, refresh) {
    this.type = type;
    this.debounce = debounce;
    this.refresh = refresh;
    this.data = new Signal();

    this._adapter = adapter;
    this._last_update = null;
  }

  _notify(ts, val) {
    if (!this._last_update || 
         ts - this._last_update > 1000*this.debounce) {
      this._last_update = ts;
      this.data.dispatch(val);
    }
  }

  async change(debounce, refresh, timeout) {
    this.debounce = debounce;
    this.refresh = refresh;
    await this._adapter._recalculate(timeout);
  }

  async cancel(timeout=1) {
    if (!this._adapter) return;
    this._adapter._subs.delete(this);
    await this._adapter._recalculate(timeout);
    this._adapter = null;
  }

  poll() {
    this._last_update = null;
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

    this._debounce = null;
    this._refresh = null;
  }

  async _recalculate(timeout) {
    var debounce = Number.POSITIVE_INFINITY;
    var refresh = Number.POSITIVE_INFINITY;
    for (let s of this._subs) {
      debounce = Math.min(s.debounce, debounce);
      refresh = Math.min(s.refresh, refresh)
    }
    if (this._subs.size == 0) {
      debounce = null;
      refresh = null;
    }
    if (debounce != this._debounce || 
       refresh != this._refresh) {
      this._debounce = debounce;
      this._refresh = refresh;
      if (this._subs.size == 0) {
        await this._stopSubscription(timeout);
      } else {
        this._type = await this._changeSubscription(debounce, refresh, timeout);
      }
    }
  }

  poll() {
    this._poll();
  }

  update(val) {
    var d = new Date();
    var t = d.getTime();
    for (let s of this._subs) s._notify(t, val);
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

