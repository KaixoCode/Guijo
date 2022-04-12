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
    class Dimensions;
    template<class Ty> requires (std::integral<Ty> || std::floating_point<Ty>)
    class Point {
    public:
        constexpr Point() : data{ 0, 0 } {};
        constexpr Point(auto x, auto y) : data{ static_cast<Ty>(x), static_cast<Ty>(y) } {}
        constexpr Point(const Vec2<Ty>& p) : data{ p } {}

        constexpr virtual Ty x() const { return data[0]; }
        constexpr virtual Ty y() const { return data[1]; }
        constexpr virtual Ty width() const { return data[0]; }
        constexpr virtual Ty height() const { return data[1]; }
        constexpr virtual operator Vec2<Ty>() const { return data; }
        constexpr virtual void x(const Ty& v) { data[0] = v; }
        constexpr virtual void y(const Ty& v) { data[1] = v; }
        constexpr virtual void width(const Ty& v) { data[0] = v; }
        constexpr virtual void height(const Ty& v) { data[1] = v; }

        constexpr virtual bool inside(const Dimensions<Ty>& v) const {
            return x() >= v.left() && x() <= v.right() && y() >= v.top() && y() <= v.bottom();
        }

        template<size_t I> requires (I < 2) constexpr auto& get()& { return data[I]; }
        template<size_t I> requires (I < 2) constexpr auto const& get() const& { return data[I]; }
        template<size_t I> requires (I < 2) constexpr auto&& get()&& { return std::move(data[I]); }

        template<class T> constexpr Point operator+(const T& o) const { return data + 0; }
        template<class T> constexpr Point operator-(const T& o) const { return data - 0; }
        template<class T> constexpr Point operator*(const T& o) const { return data * 0; }
        template<class T> constexpr Point operator/(const T& o) const { return data / 0; }
        template<class T> constexpr Point operator%(const T& o) const { return data % 0; }
        template<class T> constexpr Point operator&(const T& o) const { return data & 0; }
        template<class T> constexpr Point operator|(const T& o) const { return data | 0; }
        template<class T> constexpr Point operator^(const T& o) const { return data ^ 0; }

        template<class T> constexpr Point& operator+=(const T& o) { data += o; return *this; }
        template<class T> constexpr Point& operator-=(const T& o) { data -= o; return *this; }
        template<class T> constexpr Point& operator*=(const T& o) { data *= o; return *this; }
        template<class T> constexpr Point& operator/=(const T& o) { data /= o; return *this; }
        template<class T> constexpr Point& operator%=(const T& o) { data %= o; return *this; }
        template<class T> constexpr Point& operator&=(const T& o) { data &= o; return *this; }
        template<class T> constexpr Point& operator|=(const T& o) { data |= o; return *this; }
        template<class T> constexpr Point& operator^=(const T& o) { data ^= o; return *this; }

        template<class T> constexpr Point& operator=(const T& o) { data = o; return *this; }
        template<class T> constexpr bool operator==(const T& o) const { return data == o; }
        template<class T> constexpr bool operator!=(const T& o) const { return data != o; }

    private:
        Vec2<Ty> data;
    };
}

namespace std {
    template<class Ty>
    struct tuple_size<Guijo::Point<Ty>> : std::integral_constant<size_t, 2> { };
    template<std::size_t N, class Ty> requires (N < 2)
        struct tuple_element<N, Guijo::Point<Ty>> { using type = Ty; };
}

namespace Guijo {
    template<class Ty> requires (std::integral<Ty> || std::floating_point<Ty>)
    class Dimensions {
    public:
        using value_type = Ty;

        constexpr Dimensions() : data{ 0, 0, 0, 0 } {};
        constexpr Dimensions(const Vec2<Ty>& p, const Vec2<Ty>& s) : data{ p[0], p[1], s[0], s[1] } {}
        constexpr Dimensions(const Vec4<Ty>& d) : data(d) {}
        constexpr Dimensions(auto v)
            : data{ static_cast<Ty>(v), static_cast<Ty>(v), static_cast<Ty>(v), static_cast<Ty>(v) } {}
        constexpr Dimensions(auto x, auto y, auto w, auto h)
            : data{ static_cast<Ty>(x), static_cast<Ty>(y), static_cast<Ty>(w), static_cast<Ty>(h) } {}
        constexpr Dimensions(auto x, auto y, const Vec2<Ty>& p)
            : data{ static_cast<Ty>(x), static_cast<Ty>(y), p[0], p[1] } {}
        constexpr Dimensions(auto x, const Vec2<Ty>& p, auto h)
            : data{ static_cast<Ty>(x), p[0], p[1], static_cast<Ty>(h) } {}
        constexpr Dimensions(const Vec2<Ty>& p, auto w, auto h)
            : data{ p[0], p[1], static_cast<Ty>(w), static_cast<Ty>(h) } {}

        constexpr virtual Ty x() const { return data[0]; }
        constexpr virtual Ty y() const { return data[1]; }
        constexpr virtual Ty width() const { return data[2]; }
        constexpr virtual Ty height() const { return data[3]; }
        constexpr virtual Ty left() const { return x(); }
        constexpr virtual Ty top() const { return y(); }
        constexpr virtual Ty right() const { return x() + width(); }
        constexpr virtual Ty bottom() const { return y() + height(); }
        constexpr virtual Point<Ty> pos() const { return { x(), y() }; }
        constexpr virtual Point<Ty> size() const { return { width(), height() }; }
        constexpr virtual Dimensions<Ty> dimensions() const { return data; }
        constexpr virtual operator Vec4<Ty>() const { return data; }
        constexpr virtual void x(const Ty& v) { data[0] = v; }
        constexpr virtual void y(const Ty& v) { data[1] = v; }
        constexpr virtual void width(const Ty& v) { data[2] = v; }
        constexpr virtual void height(const Ty& v) { data[3] = v; }
        constexpr virtual void left(const Ty& v) { x(v); }
        constexpr virtual void top(const Ty& v) { y(v); }
        constexpr virtual void right(const Ty& v) { width(v - x()); }
        constexpr virtual void bottom(const Ty& v) { height(v - y()); }
        constexpr virtual void pos(const Point<Ty>& v) { x(v.x()), y(v.y()); }
        constexpr virtual void size(const Point<Ty>& v) { width(v.width()), height(v.height()); }
        constexpr virtual void dimensions(const Dimensions<Ty>& v) { data = v; }

        constexpr virtual Ty centerX() const { return data[0] + data[2] / 2; }
        constexpr virtual Ty centerY() const { return data[1] + data[3] / 2; }
        constexpr virtual Point<Ty> center() const { return { centerX(), centerY() }; }
        constexpr virtual Point<Ty> topLeft() const { return { left(), top() }; }
        constexpr virtual Point<Ty> topRight() const { return { right(), top() }; }
        constexpr virtual Point<Ty> bottomLeft() const { return { left(), bottom() }; }
        constexpr virtual Point<Ty> bottomRight() const { return { right(), bottom() }; }
        constexpr virtual Point<Ty> topCenter() const { return { centerX(), top() }; }
        constexpr virtual Point<Ty> bottomCenter() const { return { centerX(), bottom() }; }
        constexpr virtual Point<Ty> leftCenter() const { return { left(), centerY() }; }
        constexpr virtual Point<Ty> rightCenter() const { return { right(), centerY() }; }

        constexpr virtual bool contains(const Point<Ty>& v) const {
            return v.x() >= left() && v.x() <= right() && v.y() >= top() && v.y() <= bottom();
        }

        constexpr virtual Dimensions translate(const Point<Ty>& v) const {
            return { x() - v.x(), y() - v.y(), width(), height() };
        }

        constexpr virtual Dimensions overlap(const Dimensions& o) const {
            if (width() == -1 || height() == -1) return o;
            const Ty x1 = std::max(x(), o.x());
            const Ty y1 = std::max(y(), o.y());
            const Ty x2 = std::min(x() + width(), o.x() + o.width());
            const Ty y2 = std::min(y() + height(), o.y() + o.height());
            if (x1 > x2 || y1 > y2) return { 0, 0, 0, 0 };
            else return { x1, y1, x2 - x1, y2 - y1 };
        }

        constexpr virtual bool overlaps(const Dimensions& o) const {
            if (width() == -1 || height() == -1) return false;
            const Ty x1 = std::max(x(), o.x());
            const Ty y1 = std::max(y(), o.y());
            const Ty x2 = std::min(x() + width(), o.x() + o.width());
            const Ty y2 = std::min(y() + height(), o.y() + o.height());
            if (x1 > x2 || y1 > y2) return true;
            else return false;
        }

        template<size_t I> requires (I < 4) constexpr auto& get()& { return data[I]; }
        template<size_t I> requires (I < 4) constexpr auto const& get() const& { return data[I]; }
        template<size_t I> requires (I < 4) constexpr auto&& get()&& { return std::move(data[I]); }

        template<class T> constexpr Dimensions operator+(const T& o) const { return data + 0; }
        template<class T> constexpr Dimensions operator-(const T& o) const { return data - 0; }
        template<class T> constexpr Dimensions operator*(const T& o) const { return data * 0; }
        template<class T> constexpr Dimensions operator/(const T& o) const { return data / 0; }
        template<class T> constexpr Dimensions operator%(const T& o) const { return data % 0; }
        template<class T> constexpr Dimensions operator&(const T& o) const { return data & 0; }
        template<class T> constexpr Dimensions operator|(const T& o) const { return data | 0; }
        template<class T> constexpr Dimensions operator^(const T& o) const { return data ^ 0; }

        template<class T> constexpr Dimensions& operator+=(const T& o) { data += o; return *this; }
        template<class T> constexpr Dimensions& operator-=(const T& o) { data -= o; return *this; }
        template<class T> constexpr Dimensions& operator*=(const T& o) { data *= o; return *this; }
        template<class T> constexpr Dimensions& operator/=(const T& o) { data /= o; return *this; }
        template<class T> constexpr Dimensions& operator%=(const T& o) { data %= o; return *this; }
        template<class T> constexpr Dimensions& operator&=(const T& o) { data &= o; return *this; }
        template<class T> constexpr Dimensions& operator|=(const T& o) { data |= o; return *this; }
        template<class T> constexpr Dimensions& operator^=(const T& o) { data ^= o; return *this; }

        template<class T> constexpr Dimensions& operator=(const T& o) { data = o; return *this; }
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
