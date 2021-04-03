use std::collections::HashMap;
use std::sync::{Arc, Mutex, Weak};
use std::time::Duration;

use tokio::time::sleep;

use log::{debug, error};

use super::Device;
use crate::common::subscription::Publisher;
use crate::nodes::{Node, NodeID, Variable};
use crate::types;
use crate::value::Value;

pub struct DummyDevice {
    root: Node,
    // TODO: how do I want to associate publishers to variables? Maybe I want to make groups use
    // the interior mutability pattern for this reason.
    publishers: HashMap<NodeID, Arc<Mutex<Publisher>>>,
}

impl Device for DummyDevice {
    type Handler = ();

    fn add_publisher(&mut self, v: &Variable, p: Arc<Mutex<Publisher>>) {
        self.publishers.insert(v.info().id, p);
    }

    // This is a dummy device, so (for now) we don't care about handlers
    fn add_handler(&mut self, _v: &Variable, _p: Self::Handler) {}

    // TODO: params_stream_ptr request(io::yield_ctx&, const params& p) override;

    // TODO: subscription_ptr subscribe(io::yield_ctx& ctx,
    //     const variable* v,
    //     float min_interval, float max_interval,
    //     float timeout) override;

    // TODO: value call(io::yield_ctx& yield, action* a, value v, float timeout) override;

    // TODO: subscription_ptr subscribe(io::yield_ctx& yield,
    //     const std::vector<std::string_view>& path,
    //     float min_interval, float max_interval,
    //     float timeout) override {
    //     auto v = dynamic_cast<variable*>(tree_->from_path(path));
    //     if (!v) return nullptr;
    //     return subscribe(yield, v, min_interval, max_interval, timeout);
    // }

    // TODO: value call(io::yield_ctx& yield,
    //     const std::vector<std::string_view>& path,
    //     value v, float timeout) override {
    //     auto a = dynamic_cast<action*>(tree_->from_path(path));
    //     if (!a) return value::invalid();
    //     return call(yield, a, v, timeout);
    // }

    // TODO: bool write_data(io::yield_ctx& yield,
    //     variable* v,
    //     const std::vector<datapoint>& data) override {
    //     return false;
    // }
    // TODO: bool write_data(io::yield_ctx& yield,
    //     const std::vector<std::string_view>&,
    //     const std::vector<datapoint>& data) override {
    //     return false;
    // }

    // TODO: data_query_ptr query_data(io::yield_ctx& yield,
    //     const variable* v) override {
    //     return nullptr;
    // }
    // TODO: data_query_ptr query_data(io::yield_ctx& yield,
    //     const std::vector<std::string_view>& v) override {
    //     return nullptr;
    // }

    // TODO: static local_context_ptr create(io::yield_ctx&, io::io_context& ioc,
    //     const std::string_view& name, const std::string_view& type,
    //     const params& p);
    // };
}

fn publisher_ptr(value_type: types::Type) -> Arc<Mutex<Publisher>> {
    Arc::new(Mutex::new(Publisher::new(value_type)))
}

fn write_value(publisher: Weak<Mutex<Publisher>>, value: Value) -> Option<Value> {
    let publisher = publisher.upgrade()?;

    // Setting this to a variable before returning placates the borrow checker
    let out = match publisher.lock() {
        Ok(mut p) => {
            p.update(value);
            Some(value)
        }
        Err(e) => {
            error!("error when unlocking the publisher: {}", e);
            None
        }
    };

    out
}

impl DummyDevice {
    pub fn new() -> Self {
        let child_a = Node::variable(2, "a", "A", "", types::FLOAT);
        let child_b = Node::variable(3, "b", "B", "", types::UINT8);
        let child_c = Node::variable(4, "c", "C", "", types::enum_type("Status", &["On", "Off"]));

        let mut publishers: HashMap<NodeID, Arc<Mutex<Publisher>>> = HashMap::new();
        let a_publisher = publisher_ptr(types::FLOAT);
        publishers.insert(child_a.info().id, a_publisher.clone());

        tokio::spawn(async move {
            let publisher = Arc::downgrade(&a_publisher);
            // We don't want to keep an Arc to the publisher because we'll just terminate the loop
            // if the publisher gets dropped.
            std::mem::drop(a_publisher);

            loop {
                sleep(Duration::from_secs(2)).await;

                match write_value(publisher.clone(), 1.0.into()) {
                    None => break,
                    Some(v) => debug!("Wrote {:?}", v),
                }

                sleep(Duration::from_secs(2)).await;

                match write_value(publisher.clone(), 2.5.into()) {
                    None => break,
                    Some(v) => debug!("Wrote {:?}", v),
                }

                sleep(Duration::from_secs(2)).await;

                match write_value(publisher.clone(), 3.8.into()) {
                    None => break,
                    Some(v) => debug!("Wrote {:?}", v),
                }
            }
        });

        let root = Node::group(1, "foo", "Foo", "", vec![child_a, child_b, child_c], "", 1);

        Self {
            root,
            publishers,
        }
    }
}
