#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    template<std::size_t N, class Ty> 
        requires (std::integral<Ty> || std::floating_point<Ty>) 
    struct Vec {
        using value_type = Ty;
        using size_type = std::size_t;
    private:
        constexpr static auto m_Seq = std::make_index_sequence<N>{};
    public:
        template <size_t I> requires (I < N) constexpr auto& get()& { return m_Data[I]; }
        template <size_t I> requires (I < N) constexpr auto const& get() const& { return m_Data[I]; }
        template <size_t I> requires (I < N) constexpr auto&& get()&& { return std::move(m_Data[I]); }

        constexpr Ty& operator[](std::size_t i) { return m_Data[i]; }
        constexpr const Ty& operator[](std::size_t i) const { return m_Data[i]; }

        template<class T> constexpr Vec operator+(const T& o) const { return operate<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Vec operator-(const T& o) const { return operate<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Vec operator*(const T& o) const { return operate<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Vec operator/(const T& o) const { return operate<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Vec operator%(const T& o) const { return operate<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Vec operator&(const T& o) const { return operate<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Vec operator|(const T& o) const { return operate<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Vec operator^(const T& o) const { return operate<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Vec& operator+=(const T& o) { return modify<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator-=(const T& o) { return modify<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator*=(const T& o) { return modify<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator/=(const T& o) { return modify<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator%=(const T& o) { return modify<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator&=(const T& o) { return modify<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator|=(const T& o) { return modify<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Vec& operator^=(const T& o) { return modify<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Vec& operator=(const T& o) { return assign(o, m_Seq); }
        template<class T> constexpr bool operator==(const T& o) const { return equals(o, m_Seq); }
        template<class T> constexpr bool operator!=(const T& o) const { return !equals(o, m_Seq); }

        Ty m_Data[N];

        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Vec operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Vec{ _op(m_Data[Is], other.get<Is>())... };
        }
        template<class Op, std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Vec operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Vec{ _op(m_Data[Is], static_cast<Ty>(other))... };
        }
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Vec& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((m_Data[Is] = _op(m_Data[Is], other.get<Is>())), ...);
            return *this;
        }        
        template<class Op, std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Vec& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((m_Data[Is] = _op(m_Data[Is], static_cast<Ty>(other))), ...);
            return *this;
        }        
        template<class T, std::size_t ...Is> 
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Vec& assign(const T& other, std::index_sequence<Is...>) {
            ((m_Data[Is] = other.get<Is>()), ...);
            return *this;
        }        
        template<std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Vec& assign(const T& other, std::index_sequence<Is...>) {
            ((m_Data[Is] = static_cast<Ty>(other)), ...);
            return *this;
        }       
        template<class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((m_Data[Is] == other.get<Is>()) && ...);
        }
        template<std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr bool equals(const T& other, std::index_sequence<Is...>) const {
            return ((m_Data[Is] == static_cast<Ty>(other)) && ...);
        }
    };

    template<class Ty> using Vec2 = Vec<2, Ty>;
    template<class Ty> using Vec4 = Vec<4, Ty>;
}

namespace std {
    template<std::size_t N, class Ty>
    struct tuple_size<Guijo::Vec<N, Ty>> : std::integral_constant<size_t, N> { };
    template<std::size_t N, std::size_t S, class Ty> requires (N < S)
    struct tuple_element<N, Guijo::Vec<S, Ty>> { using type = Ty; };
}

namespace Guijo {
    template<class Ty> requires (std::integral<Ty> || std::floating_point<Ty>)
    class Dimensions {
    public:
        using value_type = Ty;

        constexpr Dimensions() : data{ 0, 0, 0, 0 } {};
        constexpr Dimensions(const Ty& x, const Ty& y, const Ty& w, const Ty& h) : data{ x, y, w, h } {}
        constexpr Dimensions(const Vec2<Ty>& p, const Vec2<Ty>& s) : data{ p[0], p[1], s[0], s[1] } {}
        constexpr Dimensions(const Vec4<Ty>& d) : data(d) {}

        constexpr virtual Ty x() const { return data[0]; }
        constexpr virtual Ty y() const { return data[1]; }
        constexpr virtual Ty width() const { return data[2]; }
        constexpr virtual Ty height() const { return data[3]; }
        constexpr virtual Ty left() const { return x(); }
        constexpr virtual Ty top() const { return y(); }
        constexpr virtual Ty right() const { return x() + width(); }
        constexpr virtual Ty bottom() const { return y() + height(); }
        constexpr virtual Vec2<Ty> pos() const { return { x(), y() }; }
        constexpr virtual Vec2<Ty> size() const { return { width(), height() }; }
        constexpr virtual Vec4<Ty> dimensions() const { return data; }
        constexpr virtual operator Vec4<Ty>() const { return data; }
        constexpr virtual void x(const Ty& v) { data[0] = v; }
        constexpr virtual void y(const Ty& v) { data[1] = v; }
        constexpr virtual void width(const Ty& v) { data[2] = v; }
        constexpr virtual void height(const Ty& v) { data[3] = v; }
        constexpr virtual void left(const Ty& v) { x(v); }
        constexpr virtual void top(const Ty& v) { y(v); }
        constexpr virtual void right(const Ty& v) { width(v - x()); }
        constexpr virtual void bottom(const Ty& v) { height(v - y()); }
        constexpr virtual void pos(const Vec2<Ty>& v) { x(v[0]), y(v[1]); }
        constexpr virtual void size(const Vec2<Ty>& v) { width(v[0]), height(v[1]); }
        constexpr virtual void dimensions(const Vec4<Ty>& v) { data = v; }
        constexpr virtual bool inside(const Vec2<Ty>& v) const {
            auto& [_x, _y] = v;
            return _x >= left() && _x <= right() && _y >= top() && _y <= bottom();
        }

        template<size_t I> requires (I < 4) constexpr auto& get()& { return data[I]; }
        template<size_t I> requires (I < 4) constexpr auto const& get() const& { data[I]; }
        template<size_t I> requires (I < 4) constexpr auto&& get()&& { return std::move(data[I]); }

        template<class T> constexpr Dimensions operator+(const T& o) const { return data + 0; }
        template<class T> constexpr Dimensions operator-(const T& o) const { return data - 0; }
        template<class T> constexpr Dimensions operator*(const T& o) const { return data * 0; }
        template<class T> constexpr Dimensions operator/(const T& o) const { return data / 0; }
        template<class T> constexpr Dimensions operator%(const T& o) const { return data % 0; }
        template<class T> constexpr Dimensions operator&(const T& o) const { return data & 0; }
        template<class T> constexpr Dimensions operator|(const T& o) const { return data | 0; }
        template<class T> constexpr Dimensions operator^(const T& o) const { return data ^ 0; }

        template<class T> constexpr Dimensions& operator+=(const T& o) { return data += o; }
        template<class T> constexpr Dimensions& operator-=(const T& o) { return data -= o; }
        template<class T> constexpr Dimensions& operator*=(const T& o) { return data *= o; }
        template<class T> constexpr Dimensions& operator/=(const T& o) { return data /= o; }
        template<class T> constexpr Dimensions& operator%=(const T& o) { return data %= o; }
        template<class T> constexpr Dimensions& operator&=(const T& o) { return data &= o; }
        template<class T> constexpr Dimensions& operator|=(const T& o) { return data |= o; }
        template<class T> constexpr Dimensions& operator^=(const T& o) { return data ^= o; }

        template<class T> constexpr Dimensions& operator=(const T& o) { return data = o; }
        template<class T> constexpr bool operator==(const T& o) const { return data == o; }
        template<class T> constexpr bool operator!=(const T& o) const { return data != o; }

    private:
        Vec4<Ty> data;
    };
}

namespace std {
    template<class Ty>
    struct tuple_size<Guijo::Dimensions<Ty>> : std::integral_constant<size_t, 4> { };
    template<std::size_t N, class Ty> requires (N < 4)
    struct tuple_element<N, Guijo::Dimensions<Ty>> { using type = Ty; };
}
