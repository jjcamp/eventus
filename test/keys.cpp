#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("works with different key types", "[keys]") {
    SECTION("std::string") {
        auto eq = event_queue<string>();
        eq.add_handler<int>("test0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire("test0", 3);
    }

    SECTION("int") {
        auto eq = event_queue<int>();
        eq.add_handler<int>(1, [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire(1, 3);
    }

    SECTION("cstring") {
        auto eq = event_queue<const char*>();
        eq.add_handler<int>("test0", [](int i) {
            REQUIRE(i == 3);
        });
        eq.fire("test0", 3);
    }

    SECTION("classic enum") {
        enum a { B, C };

        auto eq = event_queue<a>();
        eq.add_handler<int>(B, [](int i) {
            REQUIRE(i == 3);
        });
        eq.add_handler<int>(C, [](int i) {
            REQUIRE(i == 4);
        });
        eq.fire(B, 3);
        eq.fire(C, 4);
    }

    /* Does not yet work
    SECTION("scoped enum") {
        enum class a { B, C };

        auto eq = event_queue<a>();
        eq.add_handler<int>(a::B, [](int i) {
            REQUIRE(i == 3);
        });
        eq.add_handler<int>(a::C, [](int i) {
            REQUIRE(i == 4);
        });
        eq.fire(a::B, 3);
        eq.fire(a::C, 4);
    }
    */
}

