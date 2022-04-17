#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Pointer.hpp"
#include "Guijo/Utils/Utils.hpp"

namespace Guijo {
    using State = std::int64_t;
    using StateId = std::size_t;
    
    struct StateListener {
        virtual void update(StateId id, State value) = 0;
    };

    class Object;

    struct Event {
        virtual bool forward(const Object&) const = 0;

        constexpr void handle() const { m_Handled = true; }
        constexpr bool handled() const { return m_Handled; }

    private:
        mutable bool m_Handled = false;
    };

    struct EventHandler : public Refcounted {
        virtual void handle(Object&, const Event&) const = 0;
    };

    template<auto Fun> struct TypedEventHandler : public EventHandler {
        void handle(Object& self, const Event& e) const override {
            using signature = detail::signature_t<decltype(Fun)>;
            using args = detail::function_args_t<signature>;
            // Invocable immediately (static function/lambda)
            if constexpr (detail::invocable_tuple_t<decltype(Fun), args>) {
                if constexpr (std::tuple_size_v<args> == 1) { // No self argument
                    using event_type = std::decay_t<std::tuple_element_t<0, args>>;
                    if (auto _e = dynamic_cast<const event_type*>(&e)) Fun(*_e);
                }
                else {
                    using self_type = std::decay_t<std::tuple_element_t<0, args>>;
                    using event_type = std::decay_t<std::tuple_element_t<1, args>>;
                    if (auto _self = dynamic_cast<self_type*>(&self))
                        if (auto _e = dynamic_cast<const event_type*>(&e)) Fun(*_self, *_e);
                }
            }
            else { // Otherwise member function
                using self_type = detail::member_function_type_t<decltype(Fun)>;
                using event_type = std::decay_t<std::tuple_element_t<0, args>>;
                if (auto _self = dynamic_cast<self_type*>(&self))
                    if (auto _e = dynamic_cast<const event_type*>(&e)) (_self->*Fun)(*_e);
            }
        };
    };

    struct StateHandler : public Refcounted {
        virtual State handle(Object&, const Event&, Object&, State) const = 0;
    };

    template<auto Fun> struct TypedStateHandler : public StateHandler {
        const std::size_t state{};
        TypedStateHandler(std::size_t state) : state(state) {}
        State handle(Object& self,
            const Event& e, Object& c, State matches) const override {
            using signature = detail::signature_t<decltype(Fun)>;
            using args = detail::function_args_t<signature>;
            // Invocable immediately (static function/lambda)
            if constexpr (detail::invocable_tuple_t<decltype(Fun), args>) {
                // Has matched argument at end
                if constexpr (std::same_as<State, detail::last_t<args>>) {
                    if constexpr (std::tuple_size_v<args> == 3) { // No self argument
                        using event_type = std::decay_t<std::tuple_element_t<0, args>>;
                        using component_type = std::decay_t<std::tuple_element_t<1, args>>;
                        if (auto _e = dynamic_cast<const event_type*>(&e))
                            if (auto _c = dynamic_cast<component_type*>(&c))
                                return _c->set(state, Fun(*_e, *_c, matches));
                    }
                    else {
                        using self_type = std::decay_t<std::tuple_element_t<0, args>>;
                        using event_type = std::decay_t<std::tuple_element_t<1, args>>;
                        using component_type = std::decay_t<std::tuple_element_t<2, args>>;
                        if (auto _self = dynamic_cast<self_type*>(&self))
                            if (auto _e = dynamic_cast<const event_type*>(&e))
                                if (auto _c = dynamic_cast<component_type*>(&c))
                                    return _c->set(state, Fun(*_self, *_e, *_c, matches));
                    }
                }
                else {
                    if constexpr (std::tuple_size_v<args> == 2) { // No matches argument
                        using event_type = std::decay_t<std::tuple_element_t<0, args>>;
                        using component_type = std::decay_t<std::tuple_element_t<1, args>>;
                        if (auto _e = dynamic_cast<const event_type*>(&e))
                            if (auto _c = dynamic_cast<component_type*>(&c))
                                _c->set(state, Fun(*_e, *_c));
                    }
                    else {
                        using self_type = std::decay_t<std::tuple_element_t<0, args>>;
                        using event_type = std::decay_t<std::tuple_element_t<1, args>>;
                        using component_type = std::decay_t<std::tuple_element_t<2, args>>;
                        if (auto _self = dynamic_cast<self_type*>(&self))
                            if (auto _e = dynamic_cast<const event_type*>(&e))
                                if (auto _c = dynamic_cast<component_type*>(&c))
                                    _c->set(state, Fun(*_self, *_e, *_c));
                    }
                }
            }
            else { // Otherwise member function
                using self_type = detail::member_function_type_t<decltype(Fun)>;
                using event_type = std::decay_t<std::tuple_element_t<0, args>>;
                using component_type = std::decay_t<std::tuple_element_t<1, args>>;
                if (auto _self = dynamic_cast<self_type*>(&self))
                    if (auto _e = dynamic_cast<const event_type*>(&e))
                        if (auto _c = dynamic_cast<component_type*>(&c))
                            return _c->set(state, (_self->*Fun)(*_e, *_c, matches));
            }
            return 0;
        };
    };
}