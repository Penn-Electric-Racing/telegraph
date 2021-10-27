#include "dummy_device.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/spawn.hpp>

namespace telegraph {
    dummy_device::dummy_device(io::io_context& ioc, const std::string_view& name, 
                                std::unique_ptr<node>&& tree)
                           : local_context(ioc, name, "dummy_device", 
                                    params{}, std::move(tree))  {}

    dummy_device::~dummy_device() {}
    void
    dummy_device::add_publisher(const variable* v, const publisher_ptr& p) {
        publishers_.emplace(v, p);
    }

    void
    dummy_device::add_handler(const action* a, const dummy_device::handler& h) {
        handlers_.emplace(a, h);
    }

    params_stream_ptr 
    dummy_device::request(io::yield_ctx&, const params& p) {
        return nullptr;
    }

    subscription_ptr
    dummy_device::subscribe(io::yield_ctx&, const variable* v,
                            float min_interval, float max_interval,
                            float timeout) {
        auto it = publishers_.find(v);
        if (it == publishers_.end()) return nullptr;
        auto p = it->second;
        if (!p) return nullptr;
        return p->subscribe(min_interval, max_interval);
    }

    value
    dummy_device::call(io::yield_ctx& yield, action* a,
                            value arg, float timeout) {
        auto it = handlers_.find(a);
        if ( it == handlers_.end()) return value::invalid();
        return it->second(yield, arg);
    }

   local_context_ptr 
    dummy_device::create(io::yield_ctx&, io::io_context& ioc,
            const std::string_view& name, const std::string_view& type,
            const params& p) {
        auto childA = new variable(2, "a", "A", "", value_type::Float);
        auto childB = new variable(3, "b", "B", "", value_type::Uint8);

        std::vector<std::string> labels{"On", "Off"};
        auto status_type = value_type("Status", std::move(labels));
        auto childC = new variable(4, "c", "C", "", status_type);
        
        auto childD = new variable(5, "d", "D", "", value_type::Bool);


        std::vector<std::string> test_labels = {"option1", "option2", "option3"};
        value_type test_enum = value_type("test_enum", test_labels);
        std::vector<std::string> short_labels = {"first", "second"};
        value_type short_enum = value_type("short_enum", short_labels);
        // action
        auto action1 = new action(1, "action1", "action1", "", value_type::Uint8, value_type::Uint8);
        auto action2 = new action(2, "action2", "action2", "", value_type::Bool, value_type::Bool);
        auto action3 = new action(3, "action3", "action3", "", value_type::None, value_type::Uint8);
        auto action4 = new action(4, "action4", "action4", "", test_enum, test_enum);
        auto action5 = new action(5, "action5", "action5", "", short_enum, short_enum);

        std::vector<node*> children{childA, childB, childC, childD, action1, action2, action3, action4, action5};
        auto root = std::make_unique<group>(1, "foo", "Foo", "", "", 1, std::move(children));
        auto dev = std::make_shared<dummy_device>(ioc, name, std::move(root));

        auto a_publisher = std::make_shared<publisher>(ioc, value_type::Float);
        auto b_publisher = std::make_shared<publisher>(ioc, value_type::Int32);
        auto c_publisher= std::make_shared<publisher>(ioc, status_type);
        auto d_publisher= std::make_shared<publisher>(ioc, value_type::Bool);


        dev->add_publisher(childA, a_publisher);
        dev->add_publisher(childB, b_publisher);
        dev->add_publisher(childC, c_publisher);
        dev->add_publisher(childD, d_publisher);


        // add action
        dev->add_handler(action1, [] (io::yield_ctx& yield, value val) -> value {
            return value{(uint8_t)(unwrap<uint8_t>(val) * 2)};
        });
        dev->add_handler(action2, [] (io::yield_ctx& yield, value val) -> value {
            return value{(bool) !unwrap<bool>(val)};
        });
        dev->add_handler(action3, [] (io::yield_ctx& yield, value val) -> value {
            return value{(uint8_t) 42};
        });
        dev->add_handler(action4, [] (io::yield_ctx& yield, value val) -> value {
            uint8_t num = unwrap<uint8_t>(val);
            return value(value_type::Enum, num);
        });
        dev->add_handler(action5, [] (io::yield_ctx& yield, value val) -> value {
            uint8_t num = unwrap<uint8_t>(val);
            return value(value_type::Enum, num);
        });

        // spawn data-pushing loop....
        std::weak_ptr<dummy_device> w{dev};

        // spawn loop for a
        io::spawn(ioc, [&ioc, w, a_publisher](io::yield_context yield) {
            // create a timer
            io::deadline_timer timer{ioc};
            while (true) {
                timer.expires_from_now(boost::posix_time::seconds(2));
                timer.async_wait(yield);
                { auto dev = w.lock(); if (!dev) break; }
                (*a_publisher) << value{1.0f}; 

                timer.expires_from_now(boost::posix_time::seconds(2));
                timer.async_wait(yield);
                { auto dev = w.lock(); if (!dev) break; }
                (*a_publisher) << value{2.5f};
                timer.expires_from_now(boost::posix_time::seconds(2));
                timer.async_wait(yield);
                { auto dev = w.lock(); if (!dev) break; }
                (*a_publisher) << value{3.8f};
            }
        });
        io::spawn(ioc, [&ioc, w, b_publisher](io::yield_context yield) {
            // create a timer
            io::deadline_timer timer{ioc};
            uint8_t val= 0;
            while (true) {
                timer.expires_from_now(boost::posix_time::milliseconds(5));
                timer.async_wait(yield);
                { auto dev = w.lock(); if (!dev) break; }
                (*b_publisher) << value{val};
                val++;
            }
        });

        io::spawn(ioc, [&ioc, w, d_publisher](io::yield_context yield) {
            // create a timer
            io::deadline_timer timer{ioc};
            bool val= false;
            while (true) {
                timer.expires_from_now(boost::posix_time::seconds(2));
                timer.async_wait(yield);
                { auto dev = w.lock(); if (!dev) break; }
                (*d_publisher) << value{val};
                val = !val;
            }
        });
        return dev;
    }
}