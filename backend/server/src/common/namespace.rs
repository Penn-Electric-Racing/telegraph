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
        Params::List(vec.into_iter().map(Params::Str).collect())
    }
}

// TODO: what is this used for?
// class params_stream {
// private:
//     bool closed_;
//     std::function<void(params&& p)> handler_;
//     std::function<void()> on_close_;
//     std::vector<params> queued_; // back-queue if handler not set
// public:
//     signal<> destroyed;
//     params_stream() :
//         closed_(false), handler_(),
//         on_close_(), queued_() {}
//     ~params_stream() {
//         close();
//         destroyed();
//     }

//     constexpr bool is_closed() const { return closed_; }

//     void close() {
//         if (!closed_) {
//             closed_ = true;
//             if (on_close_) on_close_();
//         }
//     }

//     void write(params&& p) {
//         if (closed_) return;
//         if (handler_) handler_(std::move(p));
//         else queued_.emplace_back(std::move(p));
//     }

//     void reset_pipe() {
//         handler_ = std::function<void(params&&)>{};
//         on_close_ = std::function<void()>{};
//     }

//     void set_pipe(const std::function<void(params&& p)>& h,
//                   const std::function<void()>& on_close) {
//         handler_ = h;
//         on_close_ = on_close;
//         for (auto& p : queued_) {
//             h(std::move(p));
//         }
//         queued_.clear();
//         if (closed_) on_close_();
//     }
// };
// using params_stream_ptr = std::shared_ptr<params_stream>;
