import r from 'rethinkdb'

export class Client {
  constructor() {
    this.conn = null;
  }

  async connect(host, port) {
    return new Promise(resolve => {
      var connected = function(err, conn) {
        if (err) throw err;
        this.conn = conn;
        resolve();
      }.bind(this);
      r.connect({host: host, port: port}, connected);
    });
  }
}
