#include "Guijo/Objects/Object.hpp"
#include "Guijo/Event/BasicEvents.hpp"

using namespace Guijo;

Object::Object() {
    event<&Object::mouseWheel>();
    // Handle Hovering state
    state<[](const MouseMove& e, EventReceiver& c, State matches) {
        bool _now = c.hitbox(e.pos) && matches == 0; // New state
        bool _prev = c.get(Hovering); // Previous state
        // If state change, send enter/exit event
        if (!_now && _prev) c.handle(MouseExit{});
        else if (_now && !_prev) c.handle(MouseEnter{});
        return _now;
    }>(Hovering);
    // Handle focused state
    state<[](const MousePress&, EventReceiver& c, State matches) {
        bool _now = c.get(Hovering) && matches == 0; // New state
        bool _prev = c.get(Focused); // Previous state
        // If state change, send focus/unfocus event
        if (!_now && _prev) c.handle(Unfocus{});
        else if (_now && !_prev) c.handle(Focus{});
        return _now;
    }>(Focused);
    state<[](const Unfocus&, EventReceiver&){ return false; }>(Focused);
    // Handle pressed state
    state<[](const MousePress&, EventReceiver& c) { return c.get(Hovering); }>(Pressed);
    state<[](const MouseRelease&, EventReceiver&) { return false; }>(Pressed);
    state<[](const MouseExit&, EventReceiver&) { return false; }>(Hovering);
    // Link all FlexBox attributes as state listeners
    link(box.size.width);
    link(box.size.height);
    link(box.min.width);
    link(box.max.height);
    link(box.margin.left);
    link(box.margin.top);
    link(box.margin.right);
    link(box.margin.bottom);
    link(box.padding.left);
    link(box.padding.top);
    link(box.padding.right);
    link(box.padding.bottom);
    link(box.flex.basis);
    link(box.flex.grow);
    link(box.flex.shrink);
}

bool Object::hitbox(Point<float> pos) const {
    if (box.overflow.x != Flex::Overflow::Visible
     || box.overflow.y != Flex::Overflow::Visible) {
        return contains(pos);
    } else {
        if (contains(pos)) return true;
        for (auto& _c : objects())
            if (_c->hitbox(pos)) return true;
        return false;
    }
}

void Object::pre(DrawContext& context) const {
    context.pushClip();
    if (box.overflow.x != Flex::Overflow::Visible
     || box.overflow.y != Flex::Overflow::Visible) {
        auto _contentBox = dimensions().inset(box.padding);
        context.clip(_contentBox);
    }
}

void Object::draw(DrawContext& context) const {
    for (auto& _c : objects()) if (_c->get(Visible)) {
        _c->pre(context);
        _c->draw(context);
        _c->post(context);
    }
}

void Object::post(DrawContext& context) const {
    context.popClip();

    if (scrollbar.x && scrollbar.x->visible) scrollbar.x->draw(context);
    if (scrollbar.y && scrollbar.y->visible) scrollbar.y->draw(context);
}

void Object::update() {
    for (auto& _c : objects()) if (_c->get(Visible)) _c->update();
}

void Object::handle(const Event& e) {
    if (scrollbar.x && scrollbar.x->visible) 
        if (e.forward(*scrollbar.x)) scrollbar.x->handle(e);
    if (scrollbar.y && scrollbar.y->visible)
        if (e.forward(*scrollbar.y)) scrollbar.y->handle(e);
    for (auto& _c : objects()) // Forward event to sub-objects
        if (_c->get(Visible)) if (e.forward(*_c)) _c->handle(e);
    for (auto& _h : m_StateHandlers) { // Handle state
        State _matches = 0;
        if (scrollbar.x && scrollbar.x->visible)
            _matches += _h->handle(*this, e, *scrollbar.x, _matches);
        if (scrollbar.y && scrollbar.y->visible)
            _matches += _h->handle(*this, e, *scrollbar.y, _matches);
        for (auto & _c : std::views::reverse(objects()))
            if (_c->get(Visible)) _matches += _h->handle(*this, e, *_c, _matches);
    }
    EventReceiver::handle(e);
}

void Object::mouseWheel(const MouseWheel& e) {
    if (scrollbar.y->visible && !e.handled()) {
        e.handle();
        scrollbar.y->scrolled = std::clamp(
            scrollbar.y->scrolled - e.amount,
            scrollbar.y->range[0], scrollbar.y->range[1]);
    }
}