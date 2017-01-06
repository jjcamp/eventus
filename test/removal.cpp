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
}
