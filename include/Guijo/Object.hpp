#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Event.hpp"
#include "Guijo/Utils.hpp"

namespace Guijo {

    struct DrawContext {};

    class Object : public Refcounted, public Dimensions<double> {
    public:
        Object();
        virtual ~Object() {}

        virtual void Draw(DrawContext& context);
        virtual void Update();

        virtual bool Hitbox(Vec2<double> pos) { return inside(pos); }

        bool has(std::size_t v) const;
        bool set(std::size_t v, bool value = true);

        template<std::derived_from<Object> Ty, class ...Args>
        Ty& emplace(Args&&...args) {
            Ty* value = new Ty{ std::forward<Args>()... };
            m_Objects.push_back(dynamic_cast<Object*>(value));
            return *value;
        }

        template<std::derived_from<Object> Ty>
        void push(Pointer<Ty> object) {
            object->remember();
            m_Objects.push_back(object);
        }

        template<auto Fun> void HandleEvent() {
            m_EventHandlers.push_back(new TypedEventHandler<Fun>{});
        }

        template<auto Fun> void HandleState(std::size_t state) {
            m_StateHandlers.push_back(new TypedStateHandler<Fun>{ state });
        }

        virtual void Handle(const Event& e);

    protected:
        std::vector<bool> m_States{};
        std::vector<Pointer<EventHandler>> m_EventHandlers{};
        std::vector<Pointer<StateHandler>> m_StateHandlers{};
        std::vector<Pointer<Object>> m_Objects{};

    public: static std::size_t NewState() { return state_counter++; }
    private: static inline std::size_t state_counter = 6;
    };
}