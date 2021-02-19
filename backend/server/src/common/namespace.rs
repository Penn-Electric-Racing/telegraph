use std::collections::HashMap;
use std::rc::Rc;
use std::cell::RefCell;

use uuid::Uuid;

use crate::nodes::*;

pub trait Namespace<T: OwnedContext> {
    fn contexts(&self) -> &HashMap<Uuid, Rc<RefCell<T>>>;
}

/// This trait represents a context that is owned by a particular namespace. All contexts should
/// really be instances of OwnedContext, but we need to split the trait like this so that we can
/// make trait objects of Context (see rustc --explain E0038).
pub trait OwnedContext : Context {
    fn get_namespace(&self) -> Rc<RefCell<dyn Namespace<Self>>>;
}

pub struct ContextInfo {
    pub headless: bool,
    pub name: String,
    pub type_str: String,
    pub params: Params,
    pub uuid: Uuid,
}

/// A Context is an object that wraps a node tree and provides various functions to interface with
/// the outside world.
pub trait Context {
    // constexpr io::io_context& get_executor() { return ioc_; }
    fn get_info(&self) -> &ContextInfo;
    fn is_headless(&self) -> bool { self.get_info().headless }
    fn get_name(&self) -> &String { &self.get_info().name }
    fn get_type(&self) -> &String { &self.get_info().type_str }
    fn get_params(&self) -> &Params { &self.get_info().params }
    fn get_uuid(&self) -> &Uuid { &self.get_info().uuid }

    fn fetch(&self) -> Rc<RefCell<Node>>;

    // virtual params_stream_ptr request(io::yield_ctx&, const params& p) = 0;

    // virtual std::shared_ptr<node> fetch(io::yield_ctx& ctx) = 0;

    // // tree manipulation functions
    // virtual subscription_ptr  subscribe(io::yield_ctx& ctx,
    //     const std::vector<std::string_view>& variable,
    //     float min_interval, float max_interval,
    //     float timeout) = 0;
    // virtual subscription_ptr  subscribe(io::yield_ctx& ctx,
    //     const variable* v,
    //     float min_interval, float max_interval,
    //     float timeout) = 0;

    // virtual value call(io::yield_ctx& ctx, action* a, value v, float timeout) = 0;
    // virtual value call(io::yield_ctx& ctx, const std::vector<std::string_view>& a,
    //     value v, float timeout) = 0;

    // virtual bool write_data(io::yield_ctx& yield, variable* v,
    //     const std::vector<datapoint>& data) = 0;
    // virtual bool write_data(io::yield_ctx& yield, const std::vector<std::string_view>& var,
    //     const std::vector<datapoint>& data) = 0;

    // virtual data_query_ptr query_data(io::yield_ctx& yield, const variable* v) = 0;
    // virtual data_query_ptr query_data(io::yield_ctx& yield, const std::vector<std::string_view>& v) = 0;
}

pub enum Params {
    Float(f64),
    Num(i64),
    Bool(bool),
    Str(String),
    Node(Rc<RefCell<Node>>),
    Ctx(Rc<RefCell<dyn Context>>),
    List(Vec<Params>),
    Map(HashMap<String, Params>),
}

impl From<Vec<String>> for Params {
    fn from(vec: Vec<String>) -> Self {
        Params::List(vec.into_iter().map(|s| Params::Str(s)).collect())
    }
}
