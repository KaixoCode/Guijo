#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

using namespace Guijo;

Object::Object() {
    set(Visible); // Always start visible
    // Handle Hovering state
    state<[](const MouseMove& e, Object& c, std::size_t matches) {
        bool _now = c.hitbox(e.pos) && matches == 0; // New state
        bool _prev = c.get(Hovering); // Previous state
        // If state change, send enter/exit event
        if (!_now && _prev) c.handle(MouseExit{});
        else if (_now && !_prev) c.handle(MouseEnter{});
        return _now;
    }>(Hovering);
    // Handle focused state
    state<[](const MousePress& e, Object& c, std::size_t matches) {
        bool _now = c.get(Hovering) && matches == 0; // New state
        bool _prev = c.get(Focused); // Previous state
        // If state change, send focus/unfocus event
        if (!_now && _prev) c.handle(Unfocus{});
        else if (_now && !_prev) c.handle(Focus{});
        return _now;
    }>(Focused);
    // Handle pressed state
    state<[](const MousePress&, Object& c) { return c.get(Hovering); }>(Pressed);
    state<[](const MouseRelease&, Object& c) { return false; }>(Pressed);
}

void Object::draw(DrawContext& context) const {
    for (auto& _c : m_Objects) if (_c->get(Visible)) _c->draw(context);
}

void Object::update() {
    for (auto& _c : m_Objects) if (_c->get(Visible)) _c->update();
}

State Object::get(StateId v) const {
    return v < m_States.size() ? m_States[v] : 0;
}

State Object::set(StateId v, State value) {
    if (v >= m_States.size()) m_States.resize(v + 1);
    return m_States[v] = value;
}

void Object::handle(const Event& e) {
    for (auto& _c : m_Objects) // Forward event to sub-objects
        if (e.forward(*_c)) _c->handle(e);
    for (auto& _h : m_StateHandlers) // Handle state
        for (std::size_t _matches = 0; auto& _c : std::views::reverse(m_Objects))
            _matches += _h->handle(*this, e, *_c, _matches);
    // Handle event in our own handlers
    for (auto& _h : m_EventHandlers) _h->handle(*this, e);
}
