use api_schema::api::{Query, Mutation, Subscription};
use async_graphql::Schema;

fn main() {
    let schema = Schema::build(Query, Mutation, Subscription).finish();
    
    // Print the schema in SDL format
    println!("{}", &schema.sdl());
}