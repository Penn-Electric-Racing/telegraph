use std::collections::HashMap;
use std::rc::{Rc, Weak};
use std::cell::RefCell;

use uuid::Uuid;

use crate::common::namespace::*;
use crate::nodes::*;

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
