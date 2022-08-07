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
        constexpr Pointer() : value(nullptr) {}
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

        constexpr explicit operator bool() const { return value != nullptr; }

        constexpr Ty* get() { return value; }
        constexpr const Ty* get() const { return value; }

        template<class T>
            requires (std::derived_from<T, Ty> || std::derived_from<Ty, T>)
        constexpr Pointer<T> as() {
            if (auto _v = dynamic_cast<T*>(value)) {
                _v->remember(); // We need to manually remember in this case!
                return Pointer<T>(_v);
            }
            return nullptr;
        }

    private:
        Ty* value;
    };
}