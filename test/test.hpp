#include "catch.hpp"

// Adds a handler to "eq", fires the event, and verifies the handler was called.
#define TEST_HANDLER_V(eq, event) \
    static bool called##__LINE__ = false; \
    (eq).add_handler((event), [](){ \
        called##__LINE__ = true; \
    }); \
    (eq).fire((event)); \
    REQUIRE(called##__LINE__ == true)

#define TEST_HANDLER(eq, event, value, handler) \
    static bool called##__LINE__ = false; \
    (eq).add_handler<decltype((value))>((event), [](decltype((value)) p){ \
        handler(p); \
        called##__LINE__ = true; \
    }); \
    (eq).fire((event), (value)); \
    REQUIRE(called##__LINE__ == true)

#define TEST_HANDLER_AUTO(value, handler) \
    auto eq = eventus::event_queue<int>(); \
    TEST_HANDLER(eq, __LINE__, value, handler)
