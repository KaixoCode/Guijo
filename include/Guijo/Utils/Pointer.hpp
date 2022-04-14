#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    struct Refcounted {
        std::size_t ref = 1;
        constexpr virtual ~Refcounted() {};
        constexpr void remember() { ++ref; }
        constexpr void forget() { if (--ref == 0) delete this; }
    };

    template<class Ty>
    struct Pointer {
        constexpr Pointer(Ty* val) : value(val) {}
        constexpr Pointer(Ty& val) : value(&val) {}
        constexpr Pointer(Pointer&& val) noexcept : value(val.value) { val.value = nullptr; }
        constexpr Pointer(const Pointer& val) : value(val.value) { if (value) value->remember(); }
        constexpr ~Pointer() { if (value) value->forget(); }

        constexpr Pointer& operator=(Ty* val) {
            if (value) value->forget();
            value = val;
            return *this;
        }

        constexpr Pointer& operator=(Ty& val) {
            if (value) value->forget(); 
            value = &val;
            return *this;
        }

        constexpr Pointer& operator=(Pointer&& val) {
            if (value) value->forget(); 
            value = val.value, val.value = nullptr;
            return *this;
        }

        constexpr Pointer& operator=(const Pointer& val) {
            if (value) value->forget(); 
            value = val.value;
            if (value) value->remember();
            return *this;
        }

        constexpr Ty* operator->() { return value; }
        constexpr const Ty* operator->() const { return value; }

        constexpr Ty& operator*() { return *value; }
        constexpr const Ty& operator*() const { return *value; }

        constexpr explicit operator Ty* () { return value; }
        constexpr explicit operator const Ty* () const { return value; }

        constexpr Ty* get() { return value; }
        constexpr const Ty* get() const { return value; }

    private:
        Ty* value;
    };
}