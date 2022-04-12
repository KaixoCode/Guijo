#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    class Color {
        constexpr static auto m_Seq = std::make_index_sequence<4>{};
    public:
        constexpr Color() : m_R(0), m_G(0), m_B(0), m_A(0) {}
        constexpr Color(float m_R, float m_G, float m_B, float m_A) : m_R(m_R), m_G(m_G), m_B(m_B), m_A(m_A) {}
        constexpr Color(float m_R, float m_G, float m_B) : m_R(m_R), m_G(m_G), m_B(m_B), m_A(255) {}
        constexpr Color(int hex) : 
            m_R(static_cast<float>((hex & 0x00FF0000) >> 16)), 
            m_G(static_cast<float>((hex & 0x0000FF00) >> 8)), 
            m_B(static_cast<float>(hex & 0x000000FF)), m_A(255.f) {}

        constexpr virtual float r() const { return m_R; }
        constexpr virtual float g() const { return m_G; }
        constexpr virtual float b() const { return m_B; }
        constexpr virtual float a() const { return m_A; }
        constexpr virtual void r(float v) { m_R = v; }
        constexpr virtual void g(float v) { m_G = v; }
        constexpr virtual void b(float v) { m_B = v; }
        constexpr virtual void a(float v) { m_A = v; }

        template <size_t I> requires (I < 4)
        auto& get()& {
            if constexpr (I == 0) return m_R;
            else if constexpr (I == 1) return m_G;
            else if constexpr (I == 2) return m_B;
            else if constexpr (I == 3) return m_A;
        }

        template <size_t I> requires (I < 4)
        auto const& get() const& {
            if constexpr (I == 0) return m_R;
            else if constexpr (I == 1) return m_G;
            else if constexpr (I == 2) return m_B;
            else if constexpr (I == 3) return m_A;
        }

        template <size_t I> requires (I < 4)
        auto&& get()&& {
            if constexpr (I == 0) return m_R;
            else if constexpr (I == 1) return m_G;
            else if constexpr (I == 2) return m_B;
            else if constexpr (I == 3) return m_A;
        }

        template<class T> constexpr Color operator+(const T& o) const { return operate<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Color operator-(const T& o) const { return operate<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Color operator*(const T& o) const { return operate<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Color operator/(const T& o) const { return operate<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Color operator%(const T& o) const { return operate<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Color operator&(const T& o) const { return operate<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Color operator|(const T& o) const { return operate<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Color operator^(const T& o) const { return operate<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Color& operator+=(const T& o) { return modify<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Color& operator-=(const T& o) { return modify<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Color& operator*=(const T& o) { return modify<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Color& operator/=(const T& o) { return modify<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Color& operator%=(const T& o) { return modify<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Color& operator&=(const T& o) { return modify<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Color& operator|=(const T& o) { return modify<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Color& operator^=(const T& o) { return modify<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Color& operator=(const T& o) { return assign(o, m_Seq); }
        template<class T> constexpr bool operator==(const T& o) const { return equals(o, m_Seq); }
        template<class T> constexpr bool operator!=(const T& o) const { return !equals(o, m_Seq); }

    private:
        float m_R{ 0 };
        float m_G{ 0 };
        float m_B{ 0 };
        float m_A{ 0 };

        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Color{ _op(get<Is>(), static_cast<float>(other.get<Is>()))... };
        }
        template<class Op, std::convertible_to<float> T, std::size_t ...Is>
        constexpr Color operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Color{ _op(get<Is>(), static_cast<float>(other))... };
        }
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((get<Is>() = _op(get<Is>(), static_cast<float>(other.get<Is>()))), ...);
            return *this;
        }
        template<class Op, std::convertible_to<float> T, std::size_t ...Is>
        constexpr Color& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((get<Is>() = _op(get<Is>(), static_cast<float>(other))), ...);
            return *this;
        }
        template<class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color& assign(const T& other, std::index_sequence<Is...>) {
            ((get<Is>() = static_cast<float>(other.get<Is>())), ...);
            return *this;
        }
        template<std::convertible_to<float> T, std::size_t ...Is>
        constexpr Color& assign(const T& other, std::index_sequence<Is...>) {
            ((get<Is>() = static_cast<float>(other)), ...);
            return *this;
        }
        template<class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((get<Is>() == other.get<Is>()) && ...);
        }
        template<std::convertible_to<float> T, std::size_t ...Is>
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((get<Is>() == other) && ...);
        }
    };
}

namespace std {
    template<> struct tuple_size<Guijo::Color> : std::integral_constant<size_t, 4> { };

    template<> struct tuple_element<0, Guijo::Color> { using type = float; };
    template<> struct tuple_element<1, Guijo::Color> { using type = float; };
    template<> struct tuple_element<2, Guijo::Color> { using type = float; };
    template<> struct tuple_element<3, Guijo::Color> { using type = float; };
}