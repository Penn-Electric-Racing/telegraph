use std::rc::Rc;

use super::Device;
use crate::common::subscription::Publisher;
use crate::nodes::Node;
use crate::types;

pub struct DummyDevice {
    root: Node,
    publishers: Vec<Rc<Publisher>>,
    // TODO: std::unordered_map<const action*, handler> handlers_;
    // TODO: using handler = std::function<void(io::yield_ctx&, value)>;
}

impl Device for DummyDevice {}


impl DummyDevice {
    pub fn new() -> Self {
        let child_a = Node::variable(2, "a", "A", "", types::FLOAT);
        let child_b = Node::variable(3, "b", "B", "", types::UINT8);
        let child_c = Node::variable(4, "c", "C", "", types::enum_type("Status", &["On", "Off"]));

        let group = Node::group(1, "foo", "Foo", "", vec![child_a, child_b, child_c], "", 1);

        Self {
            root: group,
            publishers: vec![],
        }
    }

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
