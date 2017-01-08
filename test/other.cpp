#include <string>
#include "catch.hpp"
#include "../eventus.hpp"

using namespace eventus;
using namespace std;

TEST_CASE("add_handler in handler", "[other]") {
    SECTION("new handler should not be called") {
        auto eq = event_queue<string>();
        eq.add_handler<int>("event0", [&](int i) {
            eq.add_handler<int>("event0", [](int i) {
                REQUIRE(false);
            });
            REQUIRE(i == 3);
        });
        eq.fire("event0", 3);
    }
}
