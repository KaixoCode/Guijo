#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Event/Event.hpp"
#include "Guijo/Event/Key.hpp"
#include "Guijo/Objects/Object.hpp"

namespace Guijo {
    enum class MouseButton : std::uint8_t {
        None = 0x0000,
        Left = 0x0001,
        Right = 0x0002,
        Middle = 0x0004
    };
    using MouseButtons = std::uint8_t;

    struct MouseMove : Event {
        MouseMove(Point<float> pos) : pos(pos) {}
        Point<float> pos{}; // New mouse position
        bool forward(const Object& c) const override { return c.get(Hovering); };
    };

    struct MouseDrag : Event {
        MouseDrag(Point<float> source, Point<float> pos, MouseButtons buttons, KeyMod mod)
            : source(source), pos(pos), buttons(buttons), mod(mod) {}
        Point<float> source;  // Press position of cursor
        Point<float> pos;     // Position of cursor
        MouseButtons buttons; // Buttons held down
        KeyMod mod;           // Key mods
        bool forward(const Object& c) const override { return c.get(Pressed); };
    };

    struct MouseEnter : Event {
        bool forward(const Object& c) const override { return false; }
    };

    struct MouseExit : Event {
        bool forward(const Object& c) const override { return c.get(Hovering); }
    };

    struct Focus : Event {
        bool forward(const Object& c) const override { return false; }
    };

    struct Unfocus : Event {
        bool forward(const Object& c) const override { return true; }
    };

    struct MousePress : Event {
        MousePress(Point<float> pos, MouseButton button, KeyMod mod)
            : pos(pos), button(button), mod(mod) {}
        Point<float> pos;    // Position of press
        MouseButton button; // Button pressed
        KeyMod mod;         // Key mods
        bool forward(const Object& c) const override { return c.get(Hovering); };
    };

    struct MouseClick : Event {
        MouseClick(Point<float> pos, MouseButton button, KeyMod mod)
            : pos(pos), button(button), mod(mod) {}
        Point<float> pos;    // Position of click
        MouseButton button; // Button used to click
        KeyMod mod;         // Key mods
        bool forward(const Object& c) const override { return c.get(Hovering); };
    };

    struct MouseWheel : public Event {
        MouseWheel(Point<float> pos, int amount, KeyMod mod)
            : pos(pos), amount(amount), mod(mod) {}
        Point<float> pos;     // Cursor position when mousewheel was used
        std::int16_t amount; // Amount the mousewheel was used
        KeyMod mod;          // Key mods
        bool forward(const Object& c) const override { return c.get(Hovering); };
    };

    struct MouseRelease : Event {
        MouseRelease(Point<float> pos, MouseButton button, KeyMod mod)
            : pos(pos), button(button), mod(mod) {}
        Point<float> pos;    // Position of the cursor
        MouseButton button; // Button that was released
        KeyMod mod;         // Key mods
        bool forward(const Object& c) const override { return c.get(Focused); };
    };

    struct KeyPress : public Event {
        KeyPress(KeyCode code, KeyMod mod, bool repeat)
            : keycode(code), mod(mod), repeat(repeat) {}
        KeyCode keycode; // Key code
        KeyMod mod;      // Key mods
        bool repeat;     // Is repeat key (fired when key held down)
        bool forward(const Object& c) const override { return true; };
    };

    struct KeyType : public Event {
        KeyType(char key, KeyMod mod, bool coded)
            : key(key), mod(mod), coded(coded) {}
        char key;   // Character typed
        KeyMod mod; // Key mods
        bool coded; // Is it a coded character
        bool forward(const Object& c) const override { return true; };
    };

    struct KeyRelease : public Event {
        KeyRelease(KeyCode code, KeyMod mod)
            : keycode(code), mod(mod) {}
        KeyCode keycode; // Key code of released key
        KeyMod mod;      // Key mods
        bool forward(const Object& c) const override { return true; };
    };

    constexpr bool operator==(const KeyPress& a, KeyCode b) { return (a.keycode | a.mod) == b; }
    constexpr bool operator==(const KeyRelease& a, KeyCode b) { return (a.keycode | a.mod) == b; }
    constexpr bool operator==(const KeyType& a, KeyCode b) { return (a.key | a.mod) == b; }
    constexpr bool operator==(KeyCode b, const KeyPress& a) { return (a.keycode | a.mod) == b; }
    constexpr bool operator==(KeyCode b, const KeyRelease& a) { return (a.keycode | a.mod) == b; }
    constexpr bool operator==(KeyCode b, const KeyType& a) { return (a.key | a.mod) == b; }
}