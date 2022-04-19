#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Event/Event.hpp"
#include "Guijo/Utils/Utils.hpp"
#include "Guijo/Utils/Pointer.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Graphics/Graphics.hpp"
#include "Guijo/Objects/Flex.hpp"

namespace Guijo {
    constexpr StateId Hovering = 0;
    constexpr StateId Focused  = 1; 
    constexpr StateId Pressed  = 2;
    constexpr StateId Visible  = 3;
    constexpr StateId Disabled = 4;
    constexpr StateId Selected = 5;

    class EventReceiver : public Refcounted, public Dimensions<float> {
    public:
        EventReceiver();
        virtual ~EventReceiver() = default;

        virtual bool hitbox(Point<float> pos) const;

        virtual State get(StateId v) const;
        virtual State set(StateId v, State value = 1);

        virtual void handle(const Event& e);

        template<auto Fun> void event();
        template<class Obj> void event();
        template<std::derived_from<StateListener> Ty> void link(Ty&);

    protected:
        std::vector<State> m_States{};
        std::vector<Pointer<EventHandler>> m_EventHandlers{};
        std::vector<StateListener*> m_StateListeners{};

    public: static StateId newState() { return stateCounter++; }
    private: static inline StateId stateCounter = 6;
    };

    template<auto Fun> 
    void EventReceiver::event() {
        m_EventHandlers.push_back(new TypedEventHandler<Fun>{});
    }

    template<class Obj> 
    void EventReceiver::event() {
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

    template<std::derived_from<StateListener> Ty> 
    void EventReceiver::link(Ty& val) {
        m_StateListeners.push_back(&val);
    }
}