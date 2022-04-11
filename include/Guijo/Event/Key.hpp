#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    enum Mods {
        Control      = 0x00010000, 
        Shift        = 0x00020000,
        Alt          = 0x00040000, 
        NumLock      = 0x00080000,
        CapsLock     = 0x00100000, 
        RightControl = 0x01000000,
        LeftControl  = 0x02000000, 
        RightShift   = 0x04000000, 
        LeftShift    = 0x08000000,
        LeftAlt      = 0x10000000,
        RightAlt     = 0x20000000, 
        Windows      = 0x80000000,
    };
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
            F4        = 0x73, F11       = 0x7A,
        };

        static inline const std::string& ToString(KeyCode value) {
            static std::string _ctrl = "Ctrl+";
            static std::string _shift = "Shift+";
            static std::string _alt = "Alt+";
            static std::unordered_map<KeyCode, std::string> _keys;
            _keys.reserve(33);

            std::unordered_map<KeyCode, std::string>::iterator _it;
            if ((_it = _keys.find(value)) == _keys.end()) {
                uint32_t _value = value;
                bool _c = _value & Mods::Control;
                bool _s = _value & Mods::Shift;
                bool _a = _value & Mods::Alt;
                uint16_t _l = (_value & 0x0000ffff);
                bool _f = _l >= 0x70 && _l <= 0x7B;

                std::string _str;
                _str.reserve(_c ? _ctrl.size() : 0 + _s ? _shift.size() : 0 + _a ? _alt.size() : 0 + _f ? 2 : 1);

                if (_c) _str += _ctrl;
                if (_s) _str += _shift;
                if (_a) _str += _alt;
                if (_f)
                    _str = _str + "F" + std::to_string(_l - 0x6F);
                if (_l == 188) _str += ',';
                else if (_l == ' ') _str += "SPACE";
                else if (!_f)
                    _str += static_cast<char>(_l);

                return _keys.emplace(value, std::move(_str)).first->second;
            } else {
                return _it->second;
            }
        }
    };
}