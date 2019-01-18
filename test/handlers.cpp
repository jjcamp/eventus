#include "catch.hpp"
#include "../eventus.hpp"

using namespace _eventus_util;

TEST_CASE("handlers") {
    SECTION("get_num_params") {
        STATIC_REQUIRE(get_num_params<void>() == 0);
        STATIC_REQUIRE(get_num_params<int>() == 1);
        STATIC_REQUIRE(get_num_params<int, int>() == 2);
        STATIC_REQUIRE(get_num_params<int, int, int, int>() == 4);
    }
}
