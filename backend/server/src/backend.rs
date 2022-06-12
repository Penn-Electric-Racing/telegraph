use api_schema::types::{Tree, Node, Group, Variable, Source, NodeInfo};
use api_schema::value::{Type, DataType, Value, Datapoint};
use api_schema::api::{TreeManager, Connection, Result, Error};
use async_trait::async_trait;
use std::sync::{Arc, RwLock};
use std::collections::HashMap;
use uuid::Uuid;

use futures_util::stream::StreamExt;
use tokio_socketcan::CANSocket;

pub struct Backend {
    trees: RwLock<HashMap<Uuid, Arc<dyn Connection>>>,
    live_tree: Option<Uuid>,
    sources: HashMap<Uuid, Arc<dyn SourceProvider>>
}

impl Backend {
    pub fn new(providers : Vec<Arc<dyn SourceProvider>>) -> Self {
        let mut sources = HashMap::new();
        for p in providers {
            sources.insert(p.uuid(), p);
        }
        Backend { sources, live_tree: None, trees: RwLock::new(HashMap::new()) }
    }
}

pub trait SourceProvider : Send + Sync {
    fn uuid(&self) -> Uuid;
    fn sources(&self) -> Vec<Arc<Source>>;
    fn create(&self, source_id: Uuid) -> Result<Arc<dyn Connection>>;
}

#[async_trait]
impl TreeManager for Backend {
    async fn lookup_tree<'ctx>(&'ctx self, _id: Uuid) -> Result<Option<Arc<Tree>>> {
        todo!();
    }
    async fn live_tree<'ctx>(&'ctx self) -> Result<Option<Arc<Tree>>> {
        match self.live_tree {
            None => Ok(None),
            Some(id) => {
                let trees = self.trees.read().map_err(|_| Error::new("Error getting lock"))?;
                Ok(trees.get(&id).map(|x| x.tree()))
            }
        }
    }
    async fn trees<'ctx>(&'ctx self) -> Result<Vec<Arc<Tree>>> {
        let mut v = Vec::new();
        let trees = self.trees.read().map_err(|_| Error::new("Error getting lock"))?;
        for (_, t) in trees.iter() {
            v.push(t.tree().clone())
        }
        Ok(v)
    }
    async fn sources(&self) -> Result<Vec<Arc<Source>>> {
        let mut v = Vec::new();
        for (_, s) in self.sources.iter() {
            v.extend(s.sources())
        }
        Ok(v)
    }
    // Mutations
    async fn delete_tree(&self, tree_id: Uuid) -> Result<bool> {
        let mut trees = self.trees.write().map_err(|_| Error::new("Trees locked"))?;
        trees.remove(&tree_id);
        Ok(true)
    }
    async fn create_tree(&self, provider_id: Uuid, source_id: Uuid) -> Result<Option<Arc<Tree>>> {
        let sp = self.sources.get(&provider_id).ok_or(Error::new("Unable to find source provider"))?;
        let conn = sp.create(source_id)?;
        let tree = conn.tree();
        let mut trees = self.trees.write().map_err(|_| Error::new("Trees locked"))?;
        trees.insert(tree.id, conn);
        Ok(Some(tree))
    }
    async fn set_live(&self, _tree_id: Uuid) -> Result<Option<Arc<Tree>>> {
        todo!();
    }
    async fn get_connection<'ctx>(&'ctx self, _tree_id: Uuid) -> Result<&'ctx dyn Connection> {
        todo!();
    }
}

pub struct CanProvider {
    id: Uuid,
    sources: Vec<Arc<Source>>,
    xml: perxml::AssignedCanIds
}

impl CanProvider {
    pub fn new(xml: perxml::AssignedCanIds) -> Self {
        CanProvider { id: Uuid::new_v4(), sources: Vec::new(), xml }
    }

    pub fn add_source(&mut self, s: &str) -> Uuid {
        let id = Uuid::new_v4();
        self.sources.push(Arc::new(
            Source { provider_id: self.id.clone(), source_id: id.clone(),
                     dev_type: "can".to_owned(), name: s.to_owned() }
        ));
        id
    }
}

pub struct CanConnection {
    tree: Arc<Tree>,
    last: RwLock<HashMap<Uuid, Datapoint>>,
    id_map: HashMap<u32, Vec<(Type, Uuid)>>
}

fn insert_nodes(id: &perxml::CanId, tree: &mut Node,
            id_map: &mut HashMap<u32, Vec<(Type, Uuid)>>) {
    let mut id_entries = Vec::new();
    for value in id.values.iter() {
        let mut n : &mut Node = tree;
        let mut s = value.access_string.split('.');
        let name = s.next_back().unwrap();
        for p in s {
            match n {
            Node::Group(g) => {
                g.children.push(Node::Group(Group::new(p.to_owned())));
                let i = g.children.len() - 1;
                n = &mut g.children[i];
            },
            _ => panic!()
            }
        }
        // n now points to the last one
        let data_type = Type::new(DataType::parse_perxml(&value.ty), value.unit.clone(),
                value.enum_values.clone().map(|x| x.split(".").map(|x| x.to_owned()).collect()));
        let info = NodeInfo::new(name.to_owned(), value.name.clone(), value.description.clone());
        id_entries.push((data_type.clone(), info.id.clone()));
        let node = Node::Variable(Variable { info, data_type});
        match n {
        Node::Group(g) => {
            g.children.push(node);
        },
        _ => panic!()
        }
    }
    id_map.insert(id.id, id_entries);
}

impl CanConnection {

    pub fn new(dev: &str, ids: &perxml::AssignedCanIds) -> Result<Arc<CanConnection>> {
        let mut root = Node::Group(Group::root());
        let mut id_map = HashMap::new();
        for can_id in ids.ids.iter() {
            insert_nodes(can_id, &mut root, &mut id_map);
        }
        let conn = Arc::new(CanConnection { 
            tree: Arc::new(Tree {
                id : Uuid::new_v4(),
                name: "can_tree".to_string(),
                root
            }),
            last: RwLock::new(HashMap::new()),
            id_map
         });
        let mut socket_rx = CANSocket::open(dev)?;
        let c = conn.clone();
        tokio::spawn(async move {
            while let Some(Ok(frame)) = socket_rx.next().await {
                c.parse(frame);
            }
        });
        Ok(conn)
    }

    fn parse(&self, frame: tokio_socketcan::CANFrame) {
        let id = frame.id();
        let elems = match self.id_map.get(&id) {
            Some(e) => e,
            None => {
                log::error!("Unrecognized can id {id}");
                return;
            }
        };
        let mut data = frame.data();
        let mut bit_offset = 0;
        let mut last = self.last.write().unwrap();
        for (t, i) in elems {
            let v : Value = t.format.parse(&mut data, &mut bit_offset).into();
            last.insert(i.clone(), Datapoint { value : v });
        }
    }
}

#[async_trait]
impl Connection for CanConnection {
    fn tree(&self) -> Arc<Tree> {
        self.tree.clone()
    }
    async fn poll(&self, var_id: Uuid) -> Result<Option<Datapoint>> {
        Ok(self.last.read().map_err(|_| Error::new(""))?.get(&var_id).cloned())
    }
    async fn send_action(&self, _action_id: Uuid, _val: Value) -> Result<Option<Datapoint>> {
        Err(Error::new("Actions not implemented"))
    }
    async fn subscribe(&self, _var_id: Uuid) -> Result<futures_lite::stream::Boxed<Datapoint>> {
        Err(Error::new("Subscriptions not implemented"))
    }
}

impl SourceProvider for CanProvider {
    fn uuid(&self) -> Uuid {
        self.id
    }
    fn sources(&self) -> Vec<Arc<Source>> {
        self.sources.clone()
    }
    fn create(&self, source_id: Uuid) -> Result<Arc<dyn Connection>> {
        for s in self.sources.iter() {
            if s.source_id == source_id {
                let a : Arc<dyn Connection> = CanConnection::new(&s.name, &self.xml)?;
                return Ok(a);
            }
        }
        return Err(Error::new("No such source"));
    }
}