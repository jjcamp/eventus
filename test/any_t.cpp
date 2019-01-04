#include <memory>
#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace _eventus_util;
using namespace std;

TEST_CASE("any_t", "[any_t]") {
    SECTION("works with a struct") {
        struct a {
            int b;
            string c;
        };

        a original { 3, "test" };

        auto lazy = any_t::create(a(original));
        auto result = any_t::cast<a>(lazy);

        REQUIRE(result.b == original.b);
        REQUIRE(result.c == original.c);
    }

    SECTION("throws bad_cast") {
        string s = "test";
        auto lazy = any_t::create(s);
        REQUIRE_THROWS_AS(any_t::cast<int>(lazy), bad_cast);
    }

    SECTION("works with move") {
        auto sp = unique_ptr<string>(new string("test"));
        auto lazy = any_t::create(move(sp));
        auto result = any_t::cast<unique_ptr<string>>(lazy);

        REQUIRE(*result == "test");
    }
}

