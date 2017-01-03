/*
* eventus
* A simple yet powerful event system.
*/
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace eventus {
    struct lazy_type {
        struct basetype {
            const std::type_info* typeinfo;
        };
        template<typename T> struct supertype : public basetype {
            T value;
        };
        template<typename T> static lazy_type create(T value);
        template<typename T> static T cast(lazy_type &c);
        std::unique_ptr<basetype> ptr;
    };

    template<typename event_type>
    class event_queue {
    public:
        event_queue() = default;

        // Adds an event handler which listens for the event and has an input parameter of type T.
        template<typename T> void add_handler(event_type event, std::function<void(T)> handler);
        // Adds an event handler which listest for the event and has no input parameter.
        void add_handler(event_type event, std::function<void()> handler);
        // Fires an event of the specified EventType, passing along the parameter of type T.
        template<typename T> void fire(event_type event, T parameter);
        // Fires an event of the specified EventType with no parameter.
        void fire(event_type event);

    private:
        // Readbility aliases
        template<typename T> using vec_func_void = std::vector<std::function<void(T)>>;
        template<typename T> using shared_vec_func_void = std::shared_ptr<vec_func_void<T>>;
        using vec_func_void_void = std::vector<std::function<void()>>;
        using shared_vec_func_void_void = std::shared_ptr<vec_func_void_void>;

        // Templated structs to allow the use of enums as keys
        template<typename T, typename ENABLE = void>
        struct key_type { typedef T type; };
        template<typename T>
        struct key_type<T, typename std::enable_if<std::is_enum<T>::value>::type> { typedef typename std::underlying_type<T>::type type; };

        std::unordered_map<event_type, lazy_type, std::hash<typename key_type<event_type>::type>> events;
    };

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::add_handler(event_type event, std::function<void(T)> handler) {
        if (events.count(event) == 0) {
            events[event] = lazy_type::create(shared_vec_func_void<T>(new vec_func_void<T>(0)));
        }
        lazy_type::cast<shared_vec_func_void<T>>(events[event])->emplace_back(handler);
    }

    template<typename event_type>
    void event_queue<event_type>::add_handler(event_type event, std::function<void()> handler) {
        if (events.count(event) == 0) {
            events[event] = lazy_type::create(shared_vec_func_void_void(new vec_func_void_void(0)));
        }
        lazy_type::cast<shared_vec_func_void_void>(events[event])->emplace_back(handler);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::fire(event_type event, T parameter) {
        if (events.count(event) == 1) {
            for (auto h : *lazy_type::cast<shared_vec_func_void<T>>(events[event]).get()) {
                h(parameter);
            }
        }
    }

    template<typename event_type>
    void event_queue<event_type>::fire(event_type event) {
        if (events.count(event) == 1) {
            for (auto h : *lazy_type::cast<shared_vec_func_void_void>(events[event]).get()) {
                h();
            }
        }
    }

    template<typename T>
    lazy_type lazy_type::create(T value) {
        auto lt = lazy_type();
        auto tinfo = supertype<T>();
        tinfo.value = value;
        tinfo.typeinfo = &typeid(value);
        lt.ptr = std::unique_ptr<supertype<T>>(new supertype<T>(tinfo));
        return lt;
    }

    template<typename T>
    T lazy_type::cast(lazy_type &c) {
        if (typeid(T) != *c.ptr->typeinfo) {
            throw std::bad_cast();
        }
        return ((supertype<T>*)c.ptr.get())->value;
    }
}

