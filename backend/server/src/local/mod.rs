use std::collections::HashMap;
use std::rc::{Rc, Weak};
use std::cell::RefCell;

use uuid::Uuid;

use crate::common::namespace::*;
use crate::nodes::*;

pub mod dummy_device;
pub use dummy_device::*;

pub struct LocalNamespace {
    contexts: HashMap<Uuid, Rc<RefCell<LocalContext>>>,
}

impl LocalNamespace {
    // TODO: implement factories
    // void register_factory(const std::string& type, const context_factory& f) {
    //     context_factories_.emplace(std::make_pair(type, f));
    // }
}

impl Namespace<LocalContext> for LocalNamespace {
    fn contexts(&self) -> &HashMap<Uuid, Rc<RefCell<LocalContext>>> {
        &self.contexts
    }
}

pub struct LocalContext {
    device: Box<dyn Device>,
    info: ContextInfo,
    namespace: Weak<RefCell<LocalNamespace>>,
    tree: Rc<RefCell<Node>>,
}

impl LocalContext {
    pub fn register(mut self, ns: &Rc<RefCell<LocalNamespace>>) -> Rc<RefCell<Self>> {
        // This shouldn't happen because we take ownership of self here, so there should be no way
        // to register us for another namespace after we have already been registered
        if matches!(self.namespace.upgrade(), Some(_)) {
            panic!("this context is already registered")
        }

        self.namespace = Rc::downgrade(ns);

        let uuid = self.info.uuid;
        let rc_self = Rc::new(RefCell::new(self));

        ns.borrow_mut().contexts.insert(uuid, rc_self.clone());

        rc_self
    }
}

impl Context for LocalContext {
    fn get_info(&self) -> &ContextInfo {
        &self.info
    }

    fn fetch(&self) -> Rc<RefCell<Node>> {
        self.tree.clone()
    }
}

impl OwnedContext for LocalContext {
    fn get_namespace(&self) -> Rc<RefCell<dyn Namespace<Self>>> {
        // This should be safe, because the namespace owns this context
        self.namespace
            .upgrade()
            .expect("somehow the namespace was destroyed before the context it owned")
    }
}

pub trait Device {
        // TODO: void add_publisher(const variable* v, const publisher_ptr& p);
        // TODO: void add_handler(const action* a, const handler& h);

        // TODO: params_stream_ptr request(io::yield_ctx&, const params& p) override;

        // TODO: subscription_ptr subscribe(io::yield_ctx& ctx,
        //         const variable* v,
        //         float min_interval, float max_interval,
        //         float timeout) override;

        // TODO: value call(io::yield_ctx& yield, action* a, value v, float timeout) override;

        // TODO: subscription_ptr subscribe(io::yield_ctx& yield,
        //         const std::vector<std::string_view>& path,
        //         float min_interval, float max_interval,
        //         float timeout) override {
        //     auto v = dynamic_cast<variable*>(tree_->from_path(path));
        //     if (!v) return nullptr;
        //     return subscribe(yield, v, min_interval, max_interval, timeout);
        // }

        // TODO: value call(io::yield_ctx& yield,
        //             const std::vector<std::string_view>& path,
        //             value v, float timeout) override {
        //     auto a = dynamic_cast<action*>(tree_->from_path(path));
        //     if (!a) return value::invalid();
        //     return call(yield, a, v, timeout);
        // }

        // TODO: bool write_data(io::yield_ctx& yield,
        //         variable* v,
        //         const std::vector<datapoint>& data) override {
        //     return false;
        // }
        // TODO: bool write_data(io::yield_ctx& yield,
        //         const std::vector<std::string_view>&,
        //         const std::vector<datapoint>& data) override {
        //     return false;
        // }

        // TODO: data_query_ptr query_data(io::yield_ctx& yield,
        //                             const variable* v) override {
        //     return nullptr;
        // }
        // TODO: data_query_ptr query_data(io::yield_ctx& yield,
        //         const std::vector<std::string_view>& v) override {
        //     return nullptr;
        // }
}


// TODO: class local_component : public local_context {
// public:
//     local_component(io::io_context& ioc, const std::string_view& name,
//                     const std::string_view&type, const params& i);

//     std::shared_ptr<node> fetch(io::yield_ctx& ctx) override { return nullptr; }

//     // tree manipulation functions
//     subscription_ptr  subscribe(io::yield_ctx& ctx,
//                             const std::vector<std::string_view>& variable,
//                             float min_interval, float max_interval,
//                             float timeout) override { return nullptr; }
//     subscription_ptr  subscribe(io::yield_ctx& ctx,
//                             const variable* v,
//                             float min_interval, float max_interval,
//                             float timeout) override { return nullptr; }
//     value call(io::yield_ctx& ctx, action* a, value v, float timeout) override { return value::invalid(); }
//     value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a,
//                         value v, float timeout) override { return value::invalid(); }

//     bool write_data(io::yield_ctx& yield, variable* v,
//                                 const std::vector<datapoint>& data) override { return false; }
//     bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
//                                 const std::vector<datapoint>& data) override { return false; }

//     data_query_ptr query_data(io::yield_ctx& yield, const variable* v) override { return nullptr; }
//     data_query_ptr query_data(io::yield_ctx& yield, const std::vector<std::string_view>& v) override { return nullptr; }
// };
