#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    class Color {
        constexpr static auto m_Seq = std::make_index_sequence<4>{};
    public:
        constexpr Color() {}
        constexpr Color(double r, double g, double b, double a) : r(r), g(g), b(b), a(a) {}
        constexpr Color(double r, double g, double b) : r(r), g(g), b(b), a(255) {}
        constexpr Color(int hex) : r((hex & 0x00FF0000) >> 16), 
            g((hex & 0x0000FF00) >> 8), b(hex & 0x000000FF), a(255) {}

        template <size_t I> requires (I < 4)
        auto& get()& {
            if constexpr (I == 0) return r;
            else if constexpr (I == 1) return g;
            else if constexpr (I == 2) return b;
            else if constexpr (I == 3) return a;
        }

        template <size_t I> requires (I < 4)
        auto const& get() const& {
            if constexpr (I == 0) return r;
            else if constexpr (I == 1) return g;
            else if constexpr (I == 2) return b;
            else if constexpr (I == 3) return a;
        }

        template <size_t I> requires (I < 4)
        auto&& get()&& {
            if constexpr (I == 0) return r;
            else if constexpr (I == 1) return g;
            else if constexpr (I == 2) return b;
            else if constexpr (I == 3) return a;
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

        double r;
        double g;
        double b;
        double a;

    private:
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Color{ _op(get<Is>(), other.get<Is>())... };
        }
        template<class Op, std::convertible_to<double> T, std::size_t ...Is>
        constexpr Color operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Color{ _op(get<Is>(), static_cast<double>(other))... };
        }
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((get<Is>() = _op(get<Is>(), other.get<Is>())), ...);
            return *this;
        }
        template<class Op, std::convertible_to<double> T, std::size_t ...Is>
        constexpr Color& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((get<Is>() = _op(get<Is>(), static_cast<double>(other))), ...);
            return *this;
        }
        template<class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Color& assign(const T& other, std::index_sequence<Is...>) {
            ((get<Is>() = other.get<Is>()), ...);
            return *this;
        }
        template<std::convertible_to<double> T, std::size_t ...Is>
        constexpr Color& assign(const T& other, std::index_sequence<Is...>) {
            ((get<Is>() = static_cast<double>(other)), ...);
            return *this;
        }
        template<class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((get<Is>() == other.get<Is>()) && ...);
        }
        template<std::convertible_to<double> T, std::size_t ...Is>
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((get<Is>() == static_cast<double>(other)) && ...);
        }
    };
}

namespace std {
    template<> struct tuple_size<Guijo::Color> : std::integral_constant<size_t, 4> { };

    template<> struct tuple_element<0, Guijo::Color> { using type = double; };
    template<> struct tuple_element<1, Guijo::Color> { using type = double; };
    template<> struct tuple_element<2, Guijo::Color> { using type = double; };
    template<> struct tuple_element<3, Guijo::Color> { using type = double; };
}