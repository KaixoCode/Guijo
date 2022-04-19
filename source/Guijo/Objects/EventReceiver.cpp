#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

using namespace Guijo;

EventReceiver::EventReceiver() {
    set(Visible); // Always start visible
}

bool EventReceiver::hitbox(Point<float> pos) const {
    return contains(pos);
}

State EventReceiver::get(StateId v) const {
    return v < m_States.size() ? m_States[v] : 0;
}

State EventReceiver::set(StateId v, State value) {
    if (v >= m_States.size()) m_States.resize(v + 1);
    if (m_States[v] != value) {
        for (auto& _l : m_StateListeners)
            _l->update(v, value);
    }
    return m_States[v] = value;
}

void EventReceiver::handle(const Event& e) {
    for (auto& _h : m_EventHandlers) _h->handle(*this, e);
}
