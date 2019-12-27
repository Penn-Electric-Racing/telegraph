#include <telegraph/utils/hocon.hpp>

#include <telegraph/context.hpp>
#include <telegraph/server.hpp>

#include <telegraph/nodes/action.hpp>
#include <telegraph/nodes/node.hpp>
#include <telegraph/nodes/variable.hpp>
#include <telegraph/nodes/group.hpp>
#include <telegraph/tree.hpp>

#include <iostream>

#include <filesystem>

using namespace telegraph;

class mock_executor : public action::executor {
public:
    void
    exec(const action* a, const value& v,
         const std::function<void(value)>& cb) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cb(value(!v.get<bool>()));
    }
};

int main(int argc, char** argv) {
    char* dir = std::getenv("BUILD_WORKSPACE_DIRECTORY");
    std::string file = (dir ? std::string(dir) : ".") + "/cpp/main/example.conf";
    hocon_parser parser;
    json j = parser.parse_file(file);

    tree* t = tree::unpack(j["root"]);
    std::cout << *t << std::endl;

    for (auto n : t->nodes()) {
        action* a;
        if ((a = dynamic_cast<action*>(n)) != nullptr) {
            a->set_executor(new mock_executor());
        }
    }

    std::set<std::shared_ptr<subscription>> subs;
    
    variable* v = dynamic_cast<variable*>((*t->get_root())["foo"]);
    v->on_subscribe.add([&subs](std::shared_ptr<subscription> sub) {
        subs.insert(sub);
        sub->on_cancel.add([&subs, sub]() {
            subs.erase(sub);
        });
    });

    context* ctx = new context("live");
    ctx->set_tree(t); // ctx now owns t and will free t on destruction

    server srv;
    srv.start("0.0.0.0:8000");

    {
        auto cl = srv.contexts.lock();
        srv.contexts.add(ctx);
    }


    int64_t count = 0;
    while(true) {
        for (auto sub : subs) {
            sub->on_data(datapoint(3, count++));
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // srv.join();
}
