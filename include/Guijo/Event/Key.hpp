#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    namespace Mods {
        enum {
            Control = 0x00010000,
            Shift = 0x00020000,
            Alt = 0x00040000,
            NumLock = 0x00080000,
            CapsLock = 0x00100000,
            RightControl = 0x01000000,
            LeftControl = 0x02000000,
            RightShift = 0x04000000,
            LeftShift = 0x08000000,
            LeftAlt = 0x10000000,
            RightAlt = 0x20000000,
            Windows = 0x80000000,
        };
    }
    using KeyMod = std::uint32_t;
    using KeyCode = std::uint32_t;

    class Key {
    public:
        enum {
            Backspace = 0x08, Escape    = 0x1B, 
            Tab       = 0x09, Delete    = 0x2e, 
            Enter     = 0x0d, Left      = 0x25, 
            Right     = 0x27, Up        = 0x26, 
            Down      = 0x28, Shift     = 0x10, 
            Control   = 0x11, Alt       = 0x12, 
            F1        = 0x70, F7        = 0x76,
            F2        = 0x71, F8        = 0x77,
            F3        = 0x72, F9        = 0x78,
            F4        = 0x73, F10       = 0x79,
            F5        = 0x74, F11       = 0x7A,
            F6        = 0x75, F12       = 0x7B,
        };

        static inline const std::string& ToString(KeyCode value) {
            constexpr static std::string_view _ctrl = "Ctrl+";
            constexpr static std::string_view _shift = "Shift+";
            constexpr static std::string_view _alt = "Alt+";
            static std::unordered_map<KeyCode, std::string> _keys;
            _keys.reserve(33);
            // Try to find value in lookup, to prevent generation each invocation
            if (auto _it = _keys.find(value); _it == _keys.end()) {
                bool _c = value & Mods::Control; // Check mods
                bool _s = value & Mods::Shift;   // Check mods
                bool _a = value & Mods::Alt;     // Check mods
                uint16_t _l = (value & 0x0000ffff); // Check actual key
                bool _f = _l >= 0x70 && _l <= 0x7B; // If F key
                // Predict the size of the string
                std::string _str;
                std::size_t _predictSize = 1;
                if (_c) _predictSize += _ctrl.size();
                if (_s) _predictSize += _shift.size();
                if (_a) _predictSize += _alt.size();
                if (_f) _predictSize += 3;
                _str.reserve(_predictSize);
                // Add mods
                if (_c) _str += _ctrl;
                if (_s) _str += _shift;
                if (_a) _str += _alt;
                if (_f) _str += "F" + std::to_string(_l - 0x6F);
                // Get actual character
                if (_l == 188) _str += "COMMA";
                else if (_l == ' ') _str += "SPACE";
                else if (!_f) _str += static_cast<char>(_l);
                // Add to lookup and return string
                return _keys.emplace(value, std::move(_str)).first->second;
            } else { // If in lookup, return result
                return _it->second; 
            }
        }
    };
}