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

namespace _eventus_util {
    struct any_t;
    template<typename T, typename ENABLE> struct handler_t;
}

namespace eventus {
    // Receives and dispatches events
    template<typename event_type> class event_queue;

    // Contains information about a handler
    template<typename event_type, typename T> class handler_info;
}

namespace _eventus_util {
    struct any_t {
        struct basetype {
            const std::type_info* typeinfo;
        };
        template<typename T> struct supertype : public basetype {
            T value;
        };
        template<typename T> static any_t create(T value);
        template<typename T> static T cast(any_t &c);
        std::unique_ptr<basetype> ptr;
    };

    template<typename T>
    any_t any_t::create(T value) {
        auto lt = any_t();
        auto tinfo = supertype<T>();
        tinfo.value = value;
        tinfo.typeinfo = &typeid(value);
        lt.ptr = std::unique_ptr<supertype<T>>(new supertype<T>(tinfo));
        return lt;
    }

    template<typename T>
    T any_t::cast(any_t &c) {
        if (typeid(T) != *c.ptr->typeinfo) {
            throw std::bad_cast();
        }
        return ((supertype<T>*)c.ptr.get())->value;
    }

    template<typename T, typename ENABLE = void>
    struct handler_t { typedef std::function<void(T)> type; };

    template<typename T>
    struct handler_t<T, typename std::enable_if<std::is_void<T>::value>::type> { typedef std::function<void()> type; };
}

namespace eventus {
    // A std::function with a void return type
    template<typename T> using handler = typename _eventus_util::handler_t<T>::type;

    template<typename event_type, typename T>
    class handler_info {

    friend event_queue<event_type>;

    private:
        const event_type _event;
        const handler<T>* _handler;

        const handler<T>* get_handler() const {
            if (_handler == nullptr)
                throw handler_removed();
            return _handler;
        }

        handler_info(const event_type& event, eventus::handler<T>* event_handler) :
            _event(event),
            _handler { event_handler } {}

    public:
        class handler_removed : std::exception {};

        const event_type& event() const { return _event; }

        const bool removed() const { return _handler != nullptr; }
    };

    template<typename event_type>
    class event_queue {
    public:
        event_queue() = default;

        // Adds an event handler which listens for the event and has an input parameter of type T.
        template<typename T> handler_info<event_type, T> add_handler(event_type event, handler<T> event_handler);

        // Adds an event handler which listens for the event and has no input parameter.
        handler_info<event_type, void> add_handler(event_type event, handler<void> event_handler);

        // Removes an event handler.
        template<typename T> void remove_handler(const handler_info<event_type, T>& info);

        // Fires an event of the specified EventType, passing along the parameter of type T.
        template<typename T> void fire(event_type event, T parameter);

        // Fires an event of the specified EventType with no parameter.
        void fire(event_type event);

    private:
        // Readability aliases
        template<typename T> using handlers = std::vector<std::unique_ptr<handler<T>>>;
        template<typename T> using ptr_handlers = std::shared_ptr<handlers<T>>;

        // Templated structs to allow the use of enums as keys
        template<typename T, typename ENABLE = void>
        struct key_type { typedef T type; };
        template<typename T>
        struct key_type<T, typename std::enable_if<std::is_enum<T>::value>::type> { typedef typename std::underlying_type<T>::type type; };

        template<typename T>
        void _fire(event_type event, std::function<void(handler<T>*)>);
        template<typename T>
        void _remove_unused(event_type event, const size_t max);

        std::unordered_map<event_type, _eventus_util::any_t, std::hash<typename key_type<event_type>::type>> events;
    };

    template<typename event_type>
    template<typename T>
    handler_info<event_type, T> event_queue<event_type>::add_handler(event_type event, handler<T> event_handler) {
        if (events.count(event) == 0) {
            events[event] = _eventus_util::any_t::create(ptr_handlers<T>(new handlers<T>(0)));
        }
        auto ptr_handler = std::unique_ptr<handler<T>>(new handler<T>(event_handler));
        auto info = handler_info<event_type, T>(event, ptr_handler.get());
        _eventus_util::any_t::cast<ptr_handlers<T>>(events[event])->emplace_back(std::move(ptr_handler));
        return info;
    }

    template<typename event_type>
    handler_info<event_type, void> event_queue<event_type>::add_handler(event_type event, handler<void> event_handler) {
        return add_handler<void>(event, event_handler);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::remove_handler(const handler_info<event_type, T>& info) {
        auto remove_handler = info.get_handler();
        auto handler_vec = _eventus_util::any_t::cast<ptr_handlers<T>>(events[info.event()]);

        for (auto& h : *handler_vec) {
            if (h.get() != remove_handler)
                continue;

            h = nullptr;
            break;
        }
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::fire(event_type event, T parameter) {
        _fire<T>(event, [&](handler<T>* h) { (*h)(parameter); });
    }

    template<typename event_type>
    void event_queue<event_type>::fire(event_type event) {
        _fire<void>(event, [](handler<void>* h) { (*h)(); });
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::_fire(event_type event, std::function<void(handler<T>*)> delegate) {
        if (events.count(event) != 1)
            return;

        auto to_remove = 0;
        for (auto& h : *_eventus_util::any_t::cast<ptr_handlers<T>>(events[event])) {
            if (h == nullptr) {
                ++to_remove;
                continue;
            }
            delegate(h.get());
        }
        _remove_unused<T>(event, to_remove);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::_remove_unused(event_type event, const size_t max) {
        if (max == 0 || events.count(event) != 1)
            return;

        auto remaining = max;
        auto handler_vec = _eventus_util::any_t::cast<ptr_handlers<T>>(events[event]);
        for (auto itr = handler_vec->begin(); itr != handler_vec->end() && remaining > 0; ++itr) {
            if (*itr == nullptr) {
                handler_vec->erase(itr);
                --itr;
                --remaining;
            }
        }
    }
}

