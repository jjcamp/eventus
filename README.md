eventus [![Build Status](https://travis-ci.org/jjcamp/eventus.svg?branch=master)](https://travis-ci.org/jjcamp/eventus)
=======
A simple yet powerful event system.

Requirements
------------
* C++11

Using eventus
-------------
Eventus is a header-only library, so the simplest way to include it is to add
`eventus.hpp` to your project.

Here is a simple example of using the API:
```c++
struct point {
    int x;
    int y;
}

auto eq = eventus::event_queue<std::string>();

auto my_handler = eq.add_handler<point>("moved", [](point p) {
    std::cout << "x: " << p.x << ", y: " << p.y << std::endl;
});

point new_location { 3, 6 };

eq.fire("moved", new_location);

eq.remove_handler(my_handler);
```
*Prints:* `x: 3, y: 6`

`auto eq = eventus::event_queue<std::string>()`: Creates an `event_queue`
instance which uses strings to name events.  Almost any type can be used here,
such as c-style strings, `int`s, and unscoped enums. `my_handler` is an object
which can be use to remove the event handler.

`eq.add_handler<point>("moved", /*lambda*/)`: Adds an event listener (the
lambda or other function) to the "moved" event.

`eq.fire("moved", new_location)`: Fires the "moved" event, passing on the
point-type `new_location` to any listening event handlers.

`eq.remove_handler(my_handler)`: Removes the handler from the event queue.

Other Useful Info
-----------------
* Nullary events and handlers use the non-templated `add_handler` function
and the unary `fire` function.

* Eventus uses runtime type information (RTTI).  Mismatched types between
firing an event and handling an event throw `std::bad_cast`.

Known Issues 
----------
* MSVC: Cannot use `const char*` event type with string literals. Use
`std::string` or another type instead.

* When targeting c++11, enums and scoped enums must derive from a type that can
  be converted to `size_t`. When using MSVC and targeting a higher standard, use
  `/Zc:__cplusplus` to turn on default c++14 behavior.


