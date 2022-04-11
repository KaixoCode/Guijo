#include "Guijo/Object.hpp"
#include "Guijo/BasicEvents.hpp"
using namespace Guijo;

Object::Object() {
    set(Visible); // Always start visible
    // Handle Hovering state
    HandleState<[](const MouseMove& e, Object& c, std::size_t matches) {
        bool now = c.Hitbox(e.pos) && matches == 0; // New state
        bool prev = c.has(Hovering); // Previous state
        // If state change, send enter/exit event
        if (!now && prev) c.Handle(MouseExit{});
        else if (now && !prev) c.Handle(MouseEnter{});
        return now;
    }>(Hovering);
    // Handle focused state
    HandleState<[](const MousePress& e, Object& c, std::size_t matches) {
        bool now = c.has(Hovering) && matches == 0; // New state
        bool prev = c.has(Focused); // Previous state
        // If state change, send focus/unfocus event
        if (!now && prev) c.Handle(Unfocus{});
        else if (now && !prev) c.Handle(Focus{});
        return now;
    }>(Focused);
    // Handle pressed state
    HandleState<[](const MousePress&, Object& c) { return c.has(Hovering); }>(Pressed);
    HandleState<[](const MouseRelease&, Object& c) { return false; }>(Pressed);
}

void Object::Draw(DrawContext& context) {
    for (auto& _c : m_Objects) if (_c->has(Visible)) _c->Draw(context);
}

void Object::Update() {
    for (auto& _c : m_Objects) if (_c->has(Visible)) _c->Update();
}

bool Object::has(std::size_t v) const {
    return v < m_States.size() ? m_States[v] : false;
}

bool Object::set(std::size_t v, bool value) {
    if (v >= m_States.size()) m_States.resize(v + 1);
    return m_States[v] = value;
}

void Object::Handle(const Event& e) {
    for (auto& h : m_StateHandlers) // Handle state
        for (std::size_t _matches = 0; auto & _c : m_Objects)
            _matches += h->Handle(*this, e, *_c, _matches);
    // Forward event to sub-objects
    for (auto& _c : m_Objects)
        if (e.Forward(*_c)) _c->Handle(e);
    // Handle event in our own handlers
    for (auto& h : m_EventHandlers) h->Handle(*this, e);
}
