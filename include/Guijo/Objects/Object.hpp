#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Event/Event.hpp"
#include "Guijo/Utils/Utils.hpp"
#include "Guijo/Utils/Pointer.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Graphics/Graphics.hpp"
#include "Guijo/Objects/Flex.hpp"

namespace Guijo {
    using State = std::int64_t;
    using StateId = std::size_t;

    constexpr StateId Hovering = 0;
    constexpr StateId Focused  = 1; 
    constexpr StateId Pressed  = 2;
    constexpr StateId Visible  = 3;
    constexpr StateId Disabled = 4;
    constexpr StateId Selected = 5;

    class Object : public Refcounted, public Dimensions<float> {
    public:
        Flex::Box box; // FlexBox sizing

        Object();
        virtual ~Object() {}

        virtual void draw(DrawContext& context) const;
        virtual void update();

        virtual bool hitbox(Point<float> pos) const { return contains(pos); }

        virtual State get(StateId v) const;
        virtual State set(StateId v, State value = 1);

        virtual void handle(const Event& e);

        virtual std::vector<Pointer<Object>>& objects() { return m_Objects; };
        virtual std::vector<Pointer<Object>> const& objects() const { return m_Objects; };

        template<std::derived_from<Object> Ty, class ...Args>
        Ty& emplace(Args&&...args);

        template<std::derived_from<Object> Ty>
        void push(Pointer<Ty> object);

        template<auto Fun> void event();
        template<class Obj> void event();
        template<auto Fun> void state(std::size_t state);

    protected:
        std::vector<State> m_States{};
        std::vector<Pointer<EventHandler>> m_EventHandlers{};
        std::vector<Pointer<StateHandler>> m_StateHandlers{};
        std::vector<Pointer<Object>> m_Objects{};

    public: static StateId newState() { return stateCounter++; }
    private: static inline StateId stateCounter = 6;
    };

    template<std::derived_from<Object> Ty, class ...Args>
    Ty& Object::emplace(Args&&...args) {
        Ty* _value = new Ty{ std::forward<Args>(args)... };
        objects().push_back(dynamic_cast<Object*>(_value));
        return *_value;
    }

    template<std::derived_from<Object> Ty>
    void Object::push(Pointer<Ty> object) {
        object->remember();
        objects().push_back(object);
    }

    template<auto Fun> 
    void Object::event() {
        m_EventHandlers.push_back(new TypedEventHandler<Fun>{});
    }

    template<class Obj> 
    void Object::event() {
        if constexpr (detail::m_FindMP<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mousePress>{});
        if constexpr (detail::m_FindMM<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseMove>{});
        if constexpr (detail::m_FindMD<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseDrag>{});
        if constexpr (detail::m_FindMC<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseClick>{});
        if constexpr (detail::m_FindMR<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseRelease>{});
        if constexpr (detail::m_FindMW<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseWheel>{});
        if constexpr (detail::m_FindME<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseEnter>{});
        if constexpr (detail::m_FindMX<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::mouseExit>{});
        if constexpr (detail::m_FindFC<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::focus>{});
        if constexpr (detail::m_FindUF<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::unfocus>{});
        if constexpr (detail::m_FindKP<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::keyPress>{});
        if constexpr (detail::m_FindKT<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::keyType>{});
        if constexpr (detail::m_FindKR<Obj>) m_EventHandlers.push_back(new TypedEventHandler<&Obj::keyRelease>{});
    }

    template<auto Fun> 
    void Object::state(std::size_t state) {
        m_StateHandlers.push_back(new TypedStateHandler<Fun>{ state });
    }
}