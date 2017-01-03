#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("lazy_type", "[lazy_type]") {
    SECTION("works with a struct") {
        struct a {
            int b;
            string c;
        };

        a original { 3, "test" };

        auto lazy = lazy_type::create(original);
        auto result = lazy_type::cast<a>(lazy);

        REQUIRE(result.b == original.b);
        REQUIRE(result.c == original.c);
    }

    SECTION("throws bad_cast") {
        string s = "test";
        auto lazy = lazy_type::create(s);
        REQUIRE_THROWS_AS(lazy_type::cast<int>(lazy), bad_cast);
    }
}

