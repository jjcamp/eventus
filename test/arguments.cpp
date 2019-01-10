#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("works with different argument types", "[arguments]") {
    SECTION("works with an int") {
        auto eq = event_queue<string>();
        eq.add_handler<int>("test0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire("test0", 3);
    }

    SECTION("works with a std::string") {
        auto eq = event_queue<string>();
        eq.add_handler<string>("test0", [](string s) {
            REQUIRE(s == "test");
        });
        eq.fire("test0", string("test"));
    }

    SECTION("works with a cstring") {
        auto eq = event_queue<string>();
        eq.add_handler<const char*>("test0", [](const char* s) {
            REQUIRE_THAT(s, Catch::Equals("test"));
        });
        eq.fire("test0", "test");
    }

    SECTION("works with a struct") {
        struct a {
            int b;
            string c;
        };

        a original { 3, "test" };

        auto eq = event_queue<string>();
        eq.add_handler<a>("test0", [=](a thing) {
            REQUIRE(thing.b == original.b);
            REQUIRE(thing.c == original.c);
        });
        eq.fire("test0", original);
    }

    SECTION("works with a void") {
        auto eq = event_queue<string>();
        eq.add_handler("test0", []() {
            REQUIRE(true);
        });
        eq.fire("test0");
    }

    SECTION("throws on different types") {
        auto eq = event_queue<string>();
        eq.add_handler("test0", []() {
            return;
        });
        REQUIRE_THROWS_AS(eq.fire("test0", "test"), bad_cast);

        eq.add_handler<int>("test1", [](int i) {
            return;
        });
        REQUIRE_THROWS_AS(eq.fire("test1"), bad_cast);

        eq.add_handler<string>("test2", [](string s) {
            return;
        });
        REQUIRE_THROWS_AS(eq.fire("test2", 3), bad_cast);
    }
}

