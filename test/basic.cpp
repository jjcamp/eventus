#define CATCH_CONFIG_MAIN

#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("basic tests", "[basic]") {
    SECTION("one event, one handler") {
        auto eq = event_queue<string>();
        eq.add_handler<int>("test0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire("test0", 3);
    }

    SECTION("one event, two handlers") {
        auto eq = event_queue<string>();
        auto handler = [](int i) {
            REQUIRE(i == 4);
        };
        eq.add_handler<int>("test0", handler);
        eq.add_handler<int>("test0", handler);
        eq.fire("test0", 4);
    }

    SECTION("two events, one handler each") {
        auto eq = event_queue<string>();
        eq.add_handler<int>("test0", [](int i) {
            REQUIRE(i == 5);
        });
        eq.add_handler<int>("test1", [](int i) {
            REQUIRE(i == 6);
        });
        eq.fire("test0", 5);
        eq.fire("test1", 6);
    }
}

