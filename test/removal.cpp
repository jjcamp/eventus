#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("remove_handler test", "[removal]") {
    SECTION("removes a handler") {
        auto eq = event_queue<string>();
        auto handler0 = eq.add_handler<int>("event0", [](int i) {
            return;
        });
        eq.remove_handler(handler0);
    }

    SECTION("doesn't call handler") {
        auto eq = event_queue<string>();
        auto handler0 = eq.add_handler<int>("event0", [](int i) {
            REQUIRE(false);
        });
        eq.remove_handler(handler0);
        eq.fire("event0", 3);
    }

    SECTION("calls the second handler only") {
        auto eq = event_queue<string>();
        auto handler0 = eq.add_handler<int>("event0", [](int i) {
            REQUIRE(false);
        });
        auto handler1 = eq.add_handler<int>("event0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.remove_handler(handler0);
        eq.fire("event0", 3);
    }

    SECTION("second handler is removed by first handler, not called") {
        INFO("test depends on the ordering of handlers, which isn't necessarily guaranteed");

        auto eq = event_queue<string>();
        handler_info<string, int>* ptr_handler1;
        auto handler0 = eq.add_handler<int>("event0", [&](int i) {
            eq.remove_handler(*ptr_handler1);
            REQUIRE(i == 3);
        });
        auto handler1 = eq.add_handler<int>("event0", [](int i) {
            REQUIRE(false);
        });
        ptr_handler1 = &handler1;
        eq.fire("event0", 3);
    }

    SECTION("first handler removes itself") {
        auto should_call = true;
        auto eq = event_queue<string>();
        handler_info<string, int>* ptr_handler0;
        // gcc + libstdc++ 4.8 are requiring explicit captures here
        auto handler0 = eq.add_handler<int>("event0", [&eq, &ptr_handler0, &should_call](int i) {
            if (should_call) {
                eq.remove_handler(*ptr_handler0);
                should_call = false;
                REQUIRE(i == 3);
                return;
            }
            REQUIRE(false);
        });
        ptr_handler0 = &handler0;
        auto handler1 = eq.add_handler<int>("event0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire("event0", 3);
        eq.fire("event0", 3);
    }
}
