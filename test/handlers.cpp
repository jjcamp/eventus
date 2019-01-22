#include "catch.hpp"
#include "../eventus.hpp"

using namespace _eventus_util;
using namespace eventus;

TEST_CASE("handlers") {
    SECTION("get_num_params") {
        STATIC_REQUIRE(get_num_params<void>() == 0);
        STATIC_REQUIRE(get_num_params<int>() == 1);
        STATIC_REQUIRE(get_num_params<int, int>() == 2);
        STATIC_REQUIRE(get_num_params<int, int, int, int>() == 4);
    }

    SECTION("wrong number of arguments") {
        // tests exercising differences between add_handler() and
        // fire() can be found in arguments.cpp
        auto eq = event_queue<int>();
        
        eq.add_handler<int>(5, [](int) { return; });
        REQUIRE_THROWS_AS(eq.add_handler(5, []{ return; }), std::invalid_argument);

        eq.add_handler(8, []{ return; });
        REQUIRE_THROWS_AS(eq.add_handler<int>(8, [](int) { return; }), std::invalid_argument);
    }
}
