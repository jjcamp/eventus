/*
* eventus
* A simple yet powerful event system.
*/
#pragma once

#include <algorithm>
#include <functional>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace _eventus_util {
    struct any_t;
    class handlers;
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
            basetype(const std::type_info* ti) : typeinfo{ti} {}
        };
        template<typename T> struct supertype : public basetype {
            T value;
            supertype(T&& val) : value{std::forward<T>(val)}, basetype(&typeid(value)) {}
        };
        template<typename T> static any_t create(T&& value);
        template<typename T> static T& cast(any_t& c);
        std::unique_ptr<basetype> ptr;
        any_t() = default;
        any_t(std::unique_ptr<basetype>&& p) : ptr{std::move(p)} {}
    };

    template<typename T>
    any_t any_t::create(T&& value) {
        return any_t(std::unique_ptr<basetype>(new supertype<T>(std::forward<T>(value))));
    }

    template<typename T>
    T& any_t::cast(any_t& c) {
        if (&typeid(T) != c.ptr->typeinfo)
            throw std::bad_cast();
        return static_cast<supertype<T>*>(c.ptr.get())->value;
    }

    template<typename T, typename ENABLE = void> struct handler_ts { typedef std::function<void(T)> type; };
    template<typename T> struct handler_ts<T, typename std::enable_if<std::is_void<T>::value>::type> {
        typedef std::function<void()> type;
    };
    template<typename T> using handler_t = typename handler_ts<T>::type;

    template<typename T>
    constexpr typename std::enable_if<!std::is_void<T>::value, int>::type get_num_params() { return 1; }
    template<typename T>
    constexpr typename std::enable_if<std::is_void<T>::value, int>::type get_num_params() { return 0; }
    template<typename T, typename S, typename...Ts>
    constexpr int get_num_params() { return sizeof...(Ts) + 2; }

    class handlers : private any_t {
    private:
        handlers(any_t&& a, int p) : any_t(std::move(a)), NUM_PARAMS{p} {};

    public:
        const int NUM_PARAMS;
        template<typename T> static handlers create();
        template<typename T> static handlers create(std::unique_ptr<handler_t<T>>&& h);
        template<typename T> std::vector<std::unique_ptr<handler_t<T>>>& get();
    };

    template<typename T>
    handlers handlers::create() {
        return handlers(any_t::create(std::vector<std::unique_ptr<handler_t<T>>>(0)), get_num_params<T>());
    }

    template<typename T>
    handlers handlers::create(std::unique_ptr<handler_t<T>>&& h) {
        auto result = handlers(any_t::create(std::vector<std::unique_ptr<handler_t<T>>>(1)), get_num_params<T>());
        result.get<T>().emplace_back(std::move(h));
        return result;
    }

    template<typename T>
    std::vector<std::unique_ptr<handler_t<T>>>& handlers::get() {
        try {
            return any_t::cast<std::vector<std::unique_ptr<handler_t<T>>>>(*this);
        }
        catch (std::bad_cast ex) {
            if (get_num_params<T>() == NUM_PARAMS)
                throw ex;
            throw std::invalid_argument("Previous operations on this event type used a different number of arguments");
        }
    }
}

namespace eventus {
    // A std::function with a void return type
    template<typename T> using handler = _eventus_util::handler_t<T>;
    using _eventus_util::handlers;

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

        void clear() { _handler = nullptr; }

        handler_info(const event_type& event, eventus::handler<T>* event_handler) :
            _event(event),
            _handler { event_handler } {}

    public:
        class handler_removed : std::exception {};

        const event_type& event() const { return _event; }

        bool removed() const { return _handler != nullptr; }
    };

    template<typename event_type>
    class event_queue {
    public:
        event_queue() = default;

        // Adds an event handler which listens for the event and has an input parameter of type T.
        template<typename T> handler_info<event_type, T> add_handler(event_type&& event, handler<T> event_handler);

        // Adds an event handler which listens for the event and has no input parameter.
        handler_info<event_type, void> add_handler(event_type&& event, handler<void> event_handler);

        // Removes an event handler.
        template<typename T> void remove_handler(handler_info<event_type, T>& info);

        // Fires an event of the specified EventType, passing along the parameter of type T.
        template<typename T> void fire(event_type&& event, T parameter);

        // Fires an event of the specified EventType with no parameter.
        void fire(event_type&& event);

    private:
        // Templated structs to allow the use of enums as keys
        template<typename T, typename ENABLE = void>
        struct key_t { typedef T type; };
        template<typename T>
        struct key_t<T, typename std::enable_if<std::is_enum<T>::value>::type> { typedef typename std::underlying_type<T>::type type; };

        template<typename T>
        void _fire(event_type&& event, void(*d)(const handler<T>*,T*), T* param);
        template<typename T>
        void _remove_unused(std::vector<std::unique_ptr<handler<T>>>& handler_vec);

        std::unordered_map<event_type, handlers, std::hash<typename key_t<event_type>::type>> events;
    };

    template<typename event_type>
    template<typename T>
    handler_info<event_type, T> event_queue<event_type>::add_handler(event_type&& event, handler<T> event_handler) {
        if (events.count(event) == 0)
            events.emplace(event, handlers::create<T>());

        auto ptr_handler = std::unique_ptr<handler<T>>(new handler<T>(event_handler));
        auto info = handler_info<event_type, T>(event, ptr_handler.get());
        events.at(event).template get<T>().emplace_back(std::move(ptr_handler));
        return info;
    }

    template<typename event_type>
    handler_info<event_type, void> event_queue<event_type>::add_handler(event_type&& event, handler<void> event_handler) {
        return add_handler<void>(std::forward<event_type>(event), event_handler);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::remove_handler(handler_info<event_type, T>& info) {
        auto remove_handler = info.get_handler();
        auto& handler_vec = events.at(info.event()).template get<T>();

        for (auto& h : handler_vec) {
            if (h.get() != remove_handler)
                continue;

            h = nullptr;
            info.clear();
            break;
        }
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::fire(event_type&& event, T parameter) {
        _fire<T>(std::forward<event_type>(event),
                 [](const handler<T>* h, T* p) { (*h)(*p); },
                 &parameter);
    }

    template<typename event_type>
    void event_queue<event_type>::fire(event_type&& event) {
        _fire<void>(std::forward<event_type>(event),
                    [](const handler<void>* h, void*) { (*h)(); },
                    nullptr);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::_fire(event_type&& event, void(*d)(const handler<T>*,T*), T* param) {
        if (events.count(event) != 1)
            return;

        auto has_removal = false;
        auto& handler_vec = events.at(event).template get<T>();
        for (const auto& h : handler_vec) {
            if (h == nullptr)
                has_removal = true;
            else
                (*d)(h.get(), param);
        }

        if (has_removal)
            _remove_unused<T>(handler_vec);
    }

    template<typename event_type>
    template<typename T>
    void event_queue<event_type>::_remove_unused(std::vector<std::unique_ptr<handler<T>>>& handler_vec) {
        std::remove(handler_vec.begin(), handler_vec.end(), nullptr);
    }
}

