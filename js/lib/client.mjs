import { Signal } from './signal.mjs'
import { Node, Type, Value } from './nodes.mjs'
import { Adapter } from './adapter.mjs'
import { Collection } from './collection.mjs'
import { Connection, Params } from './connection.mjs'
import WebSocket from 'isomorphic-ws'
import { NamespaceQuery } from './query.mjs'
import { Context, Namespace, Request } from './namespace.mjs'
import { DataQuery } from './namespace.mjs'

import { createClient, defaultExchanges, subscriptionExchange} from '@urql/core';
import * as transport_ws from 'subscriptions-transport-ws';

import { pipe, subscribe } from 'wonka';

function checkError(packet) {
  if (packet.error) throw new Error(packet.error);
  return packet;
}

// A client implements the namespace API
export class Client extends Namespace {
  constructor() {
    super();
    this._conn = null;
    this.contexts = new Collection();
    this.wsClient = new transport_ws.default.SubscriptionClient('ws://localhost:8088/subscriptions', { reconnect: true });
    this.graphqlClient = createClient({
      url: 'http://localhost:8088/graphql',
      exchanges: [
        ...defaultExchanges,
        subscriptionExchange({
          forwardSubscription: (operation) => this.wsClient.request(operation)
        }),
      ],
    });
  }

  query() { 
    var query = new NamespaceQuery(this);
    query.update(this);
    return query;
  }

  async connect(address) {
    if (this._conn) throw new Error('Already connected!');
    try {
      this._conn = new Connection(new WebSocket(address), true);
      this._conn.onClose.add(() => {
        this.contexts._clear();
        this._conn = null;
      });
      await this._conn.connect();
      await this.queryNamespaces();
    } catch (e) {
      // if we couldn't connect, set connection to null and throw an error
      this._conn = null;
      throw e;
    }
  }

  async disconnect() {
    if (this._conn) {
      await this._conn.disconnect();
      this._conn = null;
    }
  }

  // will block until the client is disconnected
  wait() {
    return new Promise((res, rej) => {
      if (!this._conn.isOpen()) res();
      else this._conn.onClose.add(res);
    });
  }

  async queryNamespaces() {
    console.log("Querying namespaces...")
    // Subscribe to the namespace changes
    const namespaceSubscription = `
      subscription Namespaces {
        namespacePacketStream {
          ... on Context {
            name
            headless
            type
            params
            uuid
          }
          ... on DestroyedContext {
            destroyedUuid
          }
        }
      }
    `;

    // TODO: store `unsubscribe` somewhere
    const { unsubscribe } = pipe(
      this.graphqlClient.subscription(namespaceSubscription),
      subscribe(result => {
        if (result == {}) return
        if (result.error) {
          console.log("GraphQL error:", result.error)
          return
        }

        if (result.data.namespacePacketStream.destroyedUuid) {

          let destroyedUuid = result.data.namespacePacketStream.destroyedUuid;
          console.log("Context destroyed:", destroyedUuid)
          this.contexts._removeUUID(destroyedUuid);

        } else if (result.data.namespacePacketStream.uuid) {

          let context = result.data.namespacePacketStream;
          console.log("Context created:", context)
          this.contexts._add(new RemoteContext(this, context.uuid, context.name, context.type, context.headless, context.params));

        } else {
          console.log(result)
        }
      })
    );
  }

  async create(name, type, params={}) {
    var msg = {
      create : {
        name: name,
        type: type,
        params: Params.pack(params)
      }
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.payload == 'success' && res.success == false) return null;
    if (!res.created) throw new Error("unexpected response: " + JSON.stringify(res));
    return this.contexts.get(res.created);
  }

  async destroy(uuid) {
    var msg = {
      destroy: uuid
    }
    var res = await this._conn.requestResponse(msg);
    checkError(res);
    if (res.success != true) 
      throw new Error("Failed to destroy context");
  }
}

class RemoteContext extends Context {
  constructor(ns, uuid, name, type, headless, params) {
    super();
    this.ns = ns;
    this.uuid = uuid;
    this.name = name;
    this.type = type;
    this.params = params;
    this.headless = headless;
    this._adapters = new Map();
  }

  // will use locally-cached copy, re-fetch otherwise
  // TODO: make locally-chaced copy 
  // a WeakRef to not leak memory
  async fetch() {
    if (!this._cached_tree) {
      let conn = this.ns._conn;
      var msg = {
        fetchTree: this.uuid
      };
      this._cached_tree = new Promise((res,rej) => {
        conn.requestResponse(msg).then((response) => {
          checkError(response);
          if (response.payload == 'success' && 
              response.success == false) {
            this._cached_tree = null;
            res(null);
            return;
          }
          if (!response.fetchedTree) throw new Error("unexpected response: " + response);
          var tree = Node.unpack(response.fetchedTree);
          tree.setContext(this);
          res(tree);
        });
      });
      return await this._cached_tree;
    } else {
      return await this._cached_tree;
    }
  }

  async subscribe(variable, debounce, refresh, timeout) {
    return await this.subscribePath(variable.path(), debounce, refresh, timeout);
  }

  async subscribePath(path, debounce, refresh, timeout, placeholder=false) {
    var key = path.join('/');
    var adapter = this._adapters.get(key);
    if (!adapter) {
      var adapter_response = null;

      var adapter_stream = null;
      var adapter_type = null;

      adapter = new Adapter(
        // poll()
        () => {
          if (adapter_stream == null) return;
          adapter_stream.send({subPoll: {}});
        },
        // sub_change();
        async (debounce, refresh, timeout) => {
          // if we need a new stream, get one
          if (adapter_response == null) {
            adapter_response = (async () => {
              let req = {
                subChange: {
                  uuid: this.uuid,
                  variable: path,
                  debounce: debounce,
                  refresh: refresh,
                  timeout: timeout,
                  placeholderOnFail: true
                }
              };
              let [response, s] = await this.ns._conn.requestStream(req);
              checkError(response);
              if (response.payload != 'subType') {
                s.close();
                adapter_response = null;
                adapter_stream = null;
                adapter_type = null;
                return [null, null];
              }
              // add listener to the stream
              s.received.add((m) => {
                if (m.cancel) adapter.close();
                else if (m.subUpdate && adapter_type) {
                  adapter.update({t: m.subUpdate.timestamp.toNumber(), v: Value.unpack(m.subUpdate.value, adapter_type)});
                }
              });
              s.start();
              adapter_stream = s;
              adapter_type = Type.unpack(response.subType);
              return [s, adapter_type];
            })();
            await adapter_response;
            if (adapter_stream == null)
              throw new Error('Failed to subscribe!');
          } else {
            // we have a stream, send an update
            await adapter_response;
            let req = {
              subChange: {
                debounce: debounce,
                refresh: refresh,
                timeout: timeout,
                placeholderOnFail: false
              }
            };
            if (!adapter_stream) throw new Error("Bad stream!");
            var res = await adapter_stream.request(req);
            if (res.payload == 'success' && !res.success) throw new Error("Subscription change failed!");
          }
          // get the type
          let [s, type] = await adapter_response;
          return type;
      }, 
      // cancel()
      async (timeout) => {
        this._adapters.set(key, null);
        if (adapter_stream == null) return;
        adapter_stream.send({cancel: timeout});
        adapter_stream.close();
        adapter_response = null;
        adapter_stream = null;
        adapter_type = null;
      });
      this._adapters.set(key, adapter);
    }
    var s = await adapter.subscribe(debounce, refresh, timeout);
    // if the adapter subscribe fails...return a placeholder
    if (!s) return null;
    return s;
  }

  async query(variable) {
    if (!this.ns || !this.ns._conn) throw new Error("Not connected!");
    var msg = {
      dataQuery: {
        uuid: this.uuid,
        path: variable.path(),
      }
    }
    var [res, stream] = await this.ns._conn.requestStream(msg);
    checkError(res);
    if (res.payload == 'success' && !res.success) {
      return null;
    }
    var valid = res.payload == 'archiveData';
    var query = new DataQuery(valid);
    if (valid) {
      console.log(res.archiveData);
    }
    stream.received.add((packet) => {
      if (packet.payload == 'archiveUpdate') {
        console.log(packet.archiveUpdate);
        query.process([]);
      } else if (packet.payload == 'cancel') {
        stream.close();
        query.close();
      }
    });
    return query;
  }

  async request(params, placeholder=false) {
    if (!this.ns || !this.ns._conn) throw new Error("Not connected!");
    var msg = {
      request: {
        uuid: this.uuid,
        params: Params.pack(params),
        placeholderOnFail: placeholder
      }
    }
    var req = new Request();
    var [res, stream] = await this.ns._conn.requestStream(msg);
    stream.received.add((packet) => {
      // parse the packet
      if (packet.requestUpdate) {
        req.process(Params.unpack(packet.requestUpdate));
      } else if (packet.cancel != undefined) {
        stream.close();
        req.close();
      }
    });
    req.closed.add(() => {
      stream.send({cancel:0});
      stream.close();
    });

    try {
      checkError(res);
      if (!res.success) return null;
      // start processing messages in the stream
      stream.start();
    } catch (e) {
      stream.close();
      throw e;
    }
    return req;
  }

  async destroy() {
    await this.ns.destroy(this.uuid);
  }
}