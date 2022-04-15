#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    template<class Type, std::size_t N, class Ty> 
    struct VecBase {
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

        template<class T> constexpr Type operator+(const T& o) const { return operate<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Type operator-(const T& o) const { return operate<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Type operator*(const T& o) const { return operate<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Type operator/(const T& o) const { return operate<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Type operator%(const T& o) const { return operate<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Type operator&(const T& o) const { return operate<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Type operator|(const T& o) const { return operate<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Type operator^(const T& o) const { return operate<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Type& operator+=(const T& o) { return modify<std::plus<>>(o, m_Seq); }
        template<class T> constexpr Type& operator-=(const T& o) { return modify<std::minus<>>(o, m_Seq); }
        template<class T> constexpr Type& operator*=(const T& o) { return modify<std::multiplies<>>(o, m_Seq); }
        template<class T> constexpr Type& operator/=(const T& o) { return modify<std::divides<>>(o, m_Seq); }
        template<class T> constexpr Type& operator%=(const T& o) { return modify<std::modulus<>>(o, m_Seq); }
        template<class T> constexpr Type& operator&=(const T& o) { return modify<std::bit_and<>>(o, m_Seq); }
        template<class T> constexpr Type& operator|=(const T& o) { return modify<std::bit_or<>>(o, m_Seq); }
        template<class T> constexpr Type& operator^=(const T& o) { return modify<std::bit_xor<>>(o, m_Seq); }

        template<class T> constexpr Type& operator=(const T& o) { return assign(o, m_Seq); }
        template<class T> constexpr bool operator==(const T& o) const { return equals(o, m_Seq); }
        template<class T> constexpr bool operator!=(const T& o) const { return !equals(o, m_Seq); }

        Ty m_Data[N];

    private:
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Type operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Type{ _op(m_Data[Is], other.get<Is>())... };
        }
        template<class Op, std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Type operate(const T& other, std::index_sequence<Is...>) const {
            constexpr Op _op{};
            return Type{ _op(m_Data[Is], static_cast<Ty>(other))... };
        }
        template<class Op, class T, std::size_t ...Is>
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Type& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((m_Data[Is] = _op(m_Data[Is], other.get<Is>())), ...);
            return static_cast<Type>(*this);
        }        
        template<class Op, std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Type& modify(const T& other, std::index_sequence<Is...>) {
            constexpr Op _op{};
            ((m_Data[Is] = _op(m_Data[Is], static_cast<Ty>(other))), ...);
            return static_cast<Type>(*this);
        }        
        template<class T, std::size_t ...Is> 
            requires requires(const T t) { (t.get<Is>(), ...); }
        constexpr Type& assign(const T& other, std::index_sequence<Is...>) {
            ((m_Data[Is] = other.get<Is>()), ...);
            return static_cast<Type>(*this);
        }        
        template<std::convertible_to<Ty> T, std::size_t ...Is>
        constexpr Type& assign(const T& other, std::index_sequence<Is...>) {
            ((m_Data[Is] = static_cast<Ty>(other)), ...);
            return static_cast<Type>(*this);
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

    template<std::size_t N, class Ty>
    struct Vec : public VecBase<Vec<N, Ty>, N, Ty> {};

    template<class Ty> using Vec2 = Vec<2, Ty>;
    template<class Ty> using Vec4 = Vec<4, Ty>;

    template<class Ty> class Dimensions;
    template<class Ty>
    class Point : public VecBase<Point<Ty>, 2, Ty> {
        using Parent = VecBase<Point<Ty>, 2, Ty>;
    public:
        using value_type = Ty;

        constexpr Point() : Parent{ {} } {};
        constexpr Point(auto x, auto y) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y) } {}
        constexpr Point(const Vec2<Ty>& p) : Parent{ p } {}

        constexpr Ty x() const { return this->get<0>(); }
        constexpr Ty y() const { return this->get<1>(); }

        template<class T> constexpr operator Vec2<T>() const { return { static_cast<T>(x()), static_cast<T>(y()) }; }
        template<class T> constexpr operator Point<T>() const { return { static_cast<T>(x()), static_cast<T>(y()) }; }

        constexpr void x(const Ty& v) { this->get<0>() = v; }
        constexpr void y(const Ty& v) { this->get<1>() = v; }

        constexpr bool inside(const Dimensions<Ty>& v) const {
            return x() >= v.left() && x() <= v.right() && y() >= v.top() && y() <= v.bottom();
        }

        constexpr Point to(const Point& p, float amt) const  {
            return { p.x() * amt + x() * (1 - amt), p.y() * amt + y() * (1 - amt) };
        }

        constexpr Ty distance(const Point& p) const {
            return std::sqrt((p.x() - x()) * (p.x() - x()) + (p.y() - y()) * (p.y() - y()));
        }
    };

    template<class Ty>
    class Size : public VecBase<Size<Ty>, 2, Ty> {
        using Parent = VecBase<Size<Ty>, 2, Ty>;
    public:
        using value_type = Ty;

        constexpr Size() : Parent{ {} } {};
        constexpr Size(auto x, auto y) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y) } {}
        constexpr Size(const Vec2<Ty>& p) : Parent{ p } {}

        constexpr Ty width() const { return this->get<0>(); }
        constexpr Ty height() const { return this->get<1>(); }

        template<class T> constexpr operator Vec2<T>() const { return { static_cast<T>(width()), static_cast<T>(height()) }; }
        template<class T> constexpr operator Size<T>() const { return { static_cast<T>(width()), static_cast<T>(height()) }; }

        constexpr void width(const Ty& v) { this->get<0>() = v; }
        constexpr void height(const Ty& v) { this->get<1>() = v; }
    };

    template<class Ty>
    class Dimensions : public VecBase<Dimensions<Ty>, 4, Ty> {
        using Parent = VecBase<Dimensions<Ty>, 4, Ty>;
    public:
        using value_type = Ty;

        constexpr Dimensions() : Parent{ {} } {};
        constexpr Dimensions(const Vec2<Ty>& p, const Vec2<Ty>& s) : Parent{ p[0], p[1], s[0], s[1] } {}
        constexpr Dimensions(const Vec4<Ty>& d) : Parent(d) {}
        constexpr Dimensions(auto v) : Parent{ static_cast<Ty>(v), static_cast<Ty>(v), static_cast<Ty>(v), static_cast<Ty>(v) } {}
        constexpr Dimensions(auto x, auto y, auto w, auto h) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y), static_cast<Ty>(w), static_cast<Ty>(h) } {}
        constexpr Dimensions(auto x, auto y, const Vec2<Ty>& p) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y), p[0], p[1] } {}
        constexpr Dimensions(auto x, const Vec2<Ty>& p, auto h) : Parent{ static_cast<Ty>(x), p[0], p[1], static_cast<Ty>(h) } {}
        constexpr Dimensions(const Vec2<Ty>& p, auto w, auto h) : Parent{ p[0], p[1], static_cast<Ty>(w), static_cast<Ty>(h) } {}
        constexpr Dimensions(Ty x, Ty y, Ty w, Ty h) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y), static_cast<Ty>(w), static_cast<Ty>(h) } {}
        constexpr Dimensions(Ty x, Ty y, const Vec2<Ty>& p) : Parent{ static_cast<Ty>(x), static_cast<Ty>(y), p[0], p[1] } {}
        constexpr Dimensions(Ty x, const Vec2<Ty>& p, Ty h) : Parent{ static_cast<Ty>(x), p[0], p[1], static_cast<Ty>(h) } {}
        constexpr Dimensions(const Vec2<Ty>& p, Ty w, Ty h) : Parent{ p[0], p[1], static_cast<Ty>(w), static_cast<Ty>(h) } {}

        constexpr virtual Ty x() const { return this->get<0>(); }
        constexpr virtual Ty y() const { return this->get<1>(); }
        constexpr virtual Ty width() const { return this->get<2>(); }
        constexpr virtual Ty height() const { return this->get<3>(); }
        constexpr virtual Ty left() const { return x(); }
        constexpr virtual Ty top() const { return y(); }
        constexpr virtual Ty right() const { return x() + width(); }
        constexpr virtual Ty bottom() const { return y() + height(); }
        constexpr virtual Point<Ty> pos() const { return { x(), y() }; }
        constexpr virtual Size<Ty> size() const { return { width(), height() }; }
        constexpr virtual Dimensions<Ty> dimensions() const { return *this; }

        template<class T> constexpr operator Vec4<T>() const { return { static_cast<T>(x()), static_cast<T>(y()), static_cast<T>(width()), static_cast<T>(height()) }; }
        template<class T> constexpr operator Dimensions<T>() const { return { static_cast<T>(x()), static_cast<T>(y()), static_cast<T>(width()), static_cast<T>(height()) }; }

        constexpr virtual void x(const Ty& v) { this->get<0>() = v; }
        constexpr virtual void y(const Ty& v) { this->get<1>() = v; }
        constexpr virtual void width(const Ty& v) { this->get<2>() = v; }
        constexpr virtual void height(const Ty& v) { this->get<3>() = v; }
        constexpr virtual void left(const Ty& v) { x(v); }
        constexpr virtual void top(const Ty& v) { y(v); }
        constexpr virtual void right(const Ty& v) { width(v - x()); }
        constexpr virtual void bottom(const Ty& v) { height(v - y()); }
        constexpr virtual void pos(const Point<Ty>& v) { x(v.x()), y(v.y()); }
        constexpr virtual void size(const Size<Ty>& v) { width(v.width()), height(v.height()); }
        constexpr virtual void dimensions(const Dimensions<Ty>& v) { *this = v; }

        constexpr Ty centerX() const { return x() + width() / 2; }
        constexpr Ty centerY() const { return y() + height() / 2; }
        constexpr Point<Ty> center() const { return { centerX(), centerY() }; }
        constexpr Point<Ty> topLeft() const { return { left(), top() }; }
        constexpr Point<Ty> topRight() const { return { right(), top() }; }
        constexpr Point<Ty> bottomLeft() const { return { left(), bottom() }; }
        constexpr Point<Ty> bottomRight() const { return { right(), bottom() }; }
        constexpr Point<Ty> topCenter() const { return { centerX(), top() }; }
        constexpr Point<Ty> bottomCenter() const { return { centerX(), bottom() }; }
        constexpr Point<Ty> leftCenter() const { return { left(), centerY() }; }
        constexpr Point<Ty> rightCenter() const { return { right(), centerY() }; }

        constexpr Dimensions inset(const Ty& amt) const {
            return { x() + amt, y() + amt, width() - 2 * amt, height() - 2 * amt };
        }

        constexpr Dimensions inset(const Ty& x, const Ty& y) const {
            return { this->x() + x, this->y() + y, width() - 2 * x, height() - 2 * y };
        }

        constexpr Dimensions inset(const Ty& left, 
            const Ty& top, const Ty& right, const Ty& bottom) const {
            return { x() + left, y() + top, width() - right - left, height() - bottom - top };
        }

        constexpr Dimensions inset(const Vec2<Ty>& amt) const {
            return { x() + amt[0], y() + amt[1], width() - 2 * amt[0], height() - 2 * amt[1] };
        }

        constexpr Dimensions inset(const Vec4<Ty>& amt) const {
            return { x() + amt[0], y() + amt[1], width() - amt[2] - amt[0], height() - amt[3] - amt[1] };
        }

        constexpr bool contains(const Point<Ty>& v) const {
            return v.x() >= left() && v.x() <= right() && v.y() >= top() && v.y() <= bottom();
        }

        constexpr Dimensions translate(const Point<Ty>& v) const {
            return { x() - v.x(), y() - v.y(), width(), height() };
        }

        constexpr Dimensions overlap(const Dimensions& o) const {
            if (width() == -1 || height() == -1) return o;
            const Ty x1 = std::max(x(), o.x());
            const Ty y1 = std::max(y(), o.y());
            const Ty x2 = std::min(x() + width(), o.x() + o.width());
            const Ty y2 = std::min(y() + height(), o.y() + o.height());
            if (x1 > x2 || y1 > y2) return { 0, 0, 0, 0 };
            else return { x1, y1, x2 - x1, y2 - y1 };
        }

        constexpr bool overlaps(const Dimensions& o) const {
            if (width() == -1 || height() == -1) return false;
            const Ty x1 = std::max(x(), o.x());
            const Ty y1 = std::max(y(), o.y());
            const Ty x2 = std::min(x() + width(), o.x() + o.width());
            const Ty y2 = std::min(y() + height(), o.y() + o.height());
            if (x1 > x2 || y1 > y2) return true;
            else return false;
        }
    };
}

namespace std {
    template<class Type, std::size_t N, class Ty>
    struct tuple_size<Guijo::VecBase<Type, N, Ty>> : std::integral_constant<size_t, N> { };
    template<class Type, std::size_t N, std::size_t S, class Ty> requires (N < S)
    struct tuple_element<N, Guijo::VecBase<Type, S, Ty>> { using type = Ty; };
    template<std::size_t N, class Ty>
    struct tuple_size<Guijo::Vec<N, Ty>> : std::integral_constant<size_t, N> { };
    template<std::size_t N, std::size_t S, class Ty> requires (N < S)
    struct tuple_element<N, Guijo::Vec<S, Ty>> { using type = Ty; };
    template<class Ty>
    struct tuple_size<Guijo::Dimensions<Ty>> : std::integral_constant<size_t, 4> { };
    template<std::size_t N, class Ty> requires (N < 4)
    struct tuple_element<N, Guijo::Dimensions<Ty>> { using type = Ty; };
    template<class Ty>
    struct tuple_size<Guijo::Point<Ty>> : std::integral_constant<size_t, 2> { };
    template<std::size_t N, class Ty> requires (N < 2)
    struct tuple_element<N, Guijo::Point<Ty>> { using type = Ty; };
    template<class Ty>
    struct tuple_size<Guijo::Size<Ty>> : std::integral_constant<size_t, 2> { };
    template<std::size_t N, class Ty> requires (N < 2)
    struct tuple_element<N, Guijo::Size<Ty>> { using type = Ty; };
}