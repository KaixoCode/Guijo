#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Utils.hpp"

namespace Guijo {
    namespace detail {
        constexpr double r2d(auto radians) {
            return 360. * radians / (std::numbers::pi_v<double> *2.);
        }
        constexpr double d2r(auto degrees) {
            return std::numbers::pi_v<double> * 2. * degrees / 360.;
        }
    }

    template<class Ty> class Radians;

    template<class Ty>
    class Degrees {
    public:
        using value_type = Ty;

        template<std::convertible_to<Ty> T>
        constexpr Degrees(const T& v = {})
            : value(static_cast<Ty>(v)) {}
        
        template<class T> constexpr Degrees(Radians<T> radians)
            : value(static_cast<Ty>(radians.degrees())) {}

        constexpr Ty degrees() const {
            return value;
        }

        constexpr Ty radians() const {
            return static_cast<Ty>(detail::d2r(value));
        }

        constexpr Ty normalized() const {
            constexpr auto _r = 360.;
            const Ty _flr1 = static_cast<Ty>(static_cast<std::int64_t>(value / _r));
            const Ty _imvl = value - _flr1 * _r + _r;
            const Ty _flr2 = static_cast<Ty>(static_cast<std::int64_t>(_imvl / _r));
            return _imvl - _flr2 * _r;
        }

        template<std::convertible_to<Ty> T>
        constexpr void degrees(const T& degrees) {
            value = static_cast<Ty>(degrees);
        }

        template<std::convertible_to<Ty> T>
        constexpr void radians(const T& radians) {
            value = static_cast<Ty>(detail::r2d(radians));
        }

        template<class T>
        constexpr explicit operator Radians<T>() const {
            return { static_cast<T>(radians()) }; 
        }

        constexpr explicit operator Ty() const { return value; }

        template<class T>
        constexpr Degrees& operator=(const Degrees<T>& other) {
            value = static_cast<Ty>(other.degrees());
            return *this;
        }

        template<class T>
        constexpr Degrees& operator=(const Radians<T>& other) {
            value = static_cast<Ty>(other.degrees());
            return *this;
        }

        template<std::convertible_to<Ty> T>
        constexpr Degrees& operator=(const T& other) {
            value = static_cast<Ty>(other);
            return *this;
        }

    private:
        Ty value;
    };

    template<class Ty>
    class Radians {
    public:
        using value_type = Ty;

        template<std::convertible_to<Ty> T>
        constexpr Radians(const T& v = {})
            : value(static_cast<Ty>(v)) {}

        template<class T> constexpr Radians(Degrees<T> degrees)
            : value(static_cast<Ty>(degrees.radians())) {}

        constexpr Ty degrees() const {
            return static_cast<Ty>(detail::r2d(value));
        }

        constexpr Ty radians() const { 
            return value;
        }

        constexpr Ty normalized() const {
            constexpr auto _r = std::numbers::pi_v<double> * 2.f;
            const Ty _flr1 = static_cast<Ty>(static_cast<std::int64_t>(value / _r));
            const Ty _imvl = value - _flr1 * _r + _r;
            const Ty _flr2 = static_cast<Ty>(static_cast<std::int64_t>(_imvl / _r));
            return _imvl - _flr2 * _r;
        }

        template<std::convertible_to<Ty> T>
        constexpr void degrees(const T& degrees) {
            value = static_cast<Ty>(detail::d2r(degrees));
        }

        template<std::convertible_to<Ty> T>
        constexpr void radians(const T& radians) {
            value = static_cast<Ty>(radians);
        }

        template<class T>
        constexpr explicit operator Degrees<T>() const {
            return { static_cast<T>(degrees()) }; 
        }

        constexpr explicit operator Ty() const { return value; }

        template<class T>
        constexpr Radians& operator=(const Degrees<T>& other) {
            value = static_cast<Ty>(other.radians());
            return *this;
        }

        template<class T>
        constexpr Radians& operator=(const Radians<T>& other) {
            value = static_cast<Ty>(other.radians());
            return *this;
        }

        template<std::convertible_to<Ty> T>
        constexpr Radians& operator=(const T& other) {
            value = static_cast<Ty>(other);
            return *this;
        }

    private:
        Ty value;
    };

    template<class Ty> Degrees(const Ty&)->Degrees<Ty>;
    template<class Ty> Radians(const Ty&)->Radians<Ty>;
    template<class Ty> using Angle = Radians<Ty>;

    template<class A>
    concept a_op = detail::template specialization<A, Radians>
                || detail::template specialization<A, Degrees>;

    template<class B, class A>
    concept b_op = detail::template specialization<B, Radians> 
                || detail::template specialization<B, Degrees>
                || std::convertible_to<B, typename A::value_type>;
                
    template<a_op A, b_op<A> B> constexpr A operator+(const A& a, const B& b) { return a_operate<std::plus<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A operator-(const A& a, const B& b) { return a_operate<std::minus<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A operator*(const A& a, const B& b) { return a_operate<std::multiplies<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A operator/(const A& a, const B& b) { return a_operate<std::divides<>>(a, b); }
             
    template<a_op A, b_op<A> B> constexpr A& operator+=(A& a, const B& b) { return a_modify<std::plus<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A& operator-=(A& a, const B& b) { return a_modify<std::minus<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A& operator*=(A& a, const B& b) { return a_modify<std::multiplies<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr A& operator/=(A& a, const B& b) { return a_modify<std::divides<>>(a, b); }
             
    template<a_op A, b_op<A> B> constexpr bool operator==(const A& a, const B& b) { return a_equality<std::equal_to<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr bool operator!=(const A& a, const B& b) { return a_equality<std::not_equal_to<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr bool operator< (const A& a, const B& b) { return a_equality<std::less<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr bool operator<=(const A& a, const B& b) { return a_equality<std::less_equal<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr bool operator> (const A& a, const B& b) { return a_equality<std::greater<>>(a, b); }
    template<a_op A, b_op<A> B> constexpr bool operator>=(const A& a, const B& b) { return a_equality<std::greater_equal<>>(a, b); }

    template<class Op, class A, class B>
    constexpr A a_operate(const A& a, const B& b) {
        constexpr Op _op{};
        if constexpr (
            detail::template specialization<B, Degrees> ||
            detail::template specialization<B, Radians>)
            return A{ Degrees{ _op(a.degrees(), b.degrees()) } };
        else return A{ Degrees{ _op(a.degrees(), A{ b }.degrees()) } };
    }

    template<class Op, class A, class B>
    constexpr A& a_modify(A& a, const B& b) {
        constexpr Op _op{};
        if constexpr (
            detail::template specialization<B, Degrees> ||
            detail::template specialization<B, Radians>)
            a.degrees(_op(a.degrees(), b.degrees()));
        else a.degrees(_op(a.degrees(), A{ b }.degrees()));
        return a;
    }

    template<class Op, class A, class B>
    constexpr bool a_equality(const A& a, const B& b) {
        constexpr Op _op{};
        if constexpr (
            detail::template specialization<B, Degrees> ||
            detail::template specialization<B, Radians>)
            return _op(a.degrees(), b.degrees());
        else return _op(a.degrees(), A{ b }.degrees());
    }
}