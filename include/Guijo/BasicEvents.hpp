#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Event.hpp"
#include "Guijo/Object.hpp"

namespace Guijo {

    struct MouseMove : Event {
        MouseMove(Vec2<double> pos) : pos(pos) {}
        Vec2<double> pos{};
        bool Forward(const Object& c) const override { return c.has(Hovering); };
    };

    struct MouseDrag : Event {
        MouseDrag(Vec2<double> pos) : pos(pos) {}
        Vec2<double> pos{};
        bool Forward(const Object& c) const override { return c.has(Pressed); };
    };

    struct MouseEnter : Event {
        bool Forward(const Object& c) const override { return false; }
    };

    struct MouseExit : Event {
        bool Forward(const Object& c) const override { return false; }
    };

    struct Focus : Event {
        bool Forward(const Object& c) const override { return false; }
    };

    struct Unfocus : Event {
        bool Forward(const Object& c) const override { return true; }
    };

    struct MousePress : Event {
        MousePress(Vec2<double> pos) : pos(pos) {}
        Vec2<double> pos{};
        bool Forward(const Object& c) const override { return c.has(Hovering); };
    };

    struct MouseClick : Event {
        MouseClick(Vec2<double> pos) : pos(pos) {}
        Vec2<double> pos{};
        bool Forward(const Object& c) const override { return c.has(Hovering); };
    };

    struct MouseRelease : Event {
        MouseRelease(Vec2<double> pos) : pos(pos) {}
        Vec2<double> pos{};
        bool Forward(const Object& c) const override { return c.has(Focused); };
    };
}