#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {
    enum States {
        Hovering = 0, Focused = 1, Pressed = 2,
        Visible = 3, Disabled = 4, Selected = 5,
    };

    template<std::size_t N, class Ty> struct Vec { Ty vals[N]; };
    template<class Ty> struct Vec<2, Ty> { Ty x, y; };
    template<class Ty> struct Vec<4, Ty> { Ty x, y, width, height; };
    template<class Ty> using Vec2 = Vec<2, Ty>;
    template<class Ty> using Vec4 = Vec<4, Ty>;

    template<class Ty>
    class Dimensions {
    public:
        using value_type = Ty;
        constexpr value_type x() const { return m_X; }
        constexpr value_type y() const { return m_Y; }
        constexpr value_type width() const { return m_Width; }
        constexpr value_type height() const { return m_Height; }
        constexpr value_type left() const { return m_X; }
        constexpr value_type top() const { return m_Y; }
        constexpr value_type right() const { return m_X + m_Width; }
        constexpr value_type bottom() const { return m_Y + m_Height; }
        constexpr Vec2<value_type> pos() const { return { m_X, m_Y }; }
        constexpr Vec2<value_type> size() const { return { m_Width, m_Height }; }
        constexpr Vec4<value_type> dimensions() const { return { m_X, m_Y, m_Width, m_Height }; }
        constexpr void x(value_type v) { m_X = v; }
        constexpr void y(value_type v) { m_Y = v; }
        constexpr void width(value_type v) { m_Width = v; }
        constexpr void height(value_type v) { m_Height = v; }
        constexpr void left(value_type v) const { m_X = v; }
        constexpr void top(value_type v) const { m_Y = v; }
        constexpr void right(value_type v) const { m_Width = v - m_X; }
        constexpr void bottom(value_type v) const { m_Height = v - m_Y; }
        constexpr void pos(Vec2<value_type> v) { m_X = v.x, m_Y = v.y; }
        constexpr void size(Vec2<value_type> v) { m_Width = v.x, m_Height = v.y; }
        constexpr void dimensions(Vec4<value_type> v) { m_X = v.x, m_Y = v.y, m_Width = v.width, m_Height = v.height; }
        constexpr bool inside(Vec2<value_type> v) const {
            return v.x >= left() && v.x <= right() && v.y >= top() && v.y <= bottom();
        }

    private:
        value_type m_X;
        value_type m_Y;
        value_type m_Width;
        value_type m_Height;
    };

    struct Refcounted {
        std::size_t ref = 1;
        constexpr virtual ~Refcounted() {};
        constexpr void remember() { ++ref; }
        constexpr void forget() { if (--ref == 0) delete this; }
    };

    template<class Ty>
    struct Pointer {
        constexpr Pointer(Ty* val) : value(val) {}
        constexpr Pointer(Pointer&& val) : value(val.value) { val.value = nullptr; }
        constexpr Pointer(const Pointer& val) : value(val.value) { value->remember(); }
        constexpr ~Pointer() { if (value) value->forget(); }

        constexpr Pointer& operator=(Ty* val) {
            value->forget(), value = val;
            return *this;
        }

        constexpr Pointer& operator=(Pointer&& val) {
            value->forget(), value = val.value, val.value = nullptr;
            return *this;
        }

        constexpr Pointer& operator=(const Pointer& val) {
            value->forget(), value = val.value, value->remember();
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

    namespace detail {
        namespace detail {
            template<class Ty> concept has_fun_op = requires(decltype(&Ty::operator()) a) { a; };
            template<class Ty> struct signature { static_assert(has_fun_op<Ty>, "Type has no function signature."); };
            template<has_fun_op Ty>
            struct signature<Ty> { using type = typename signature<decltype(&Ty::operator())>::type; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...)> { using type = R(Args...); };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) const> { using type = R(Args...) const; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...)&&> { using type = R(Args...)&&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...)&> { using type = R(Args...)&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) const&&> { using type = R(Args...) const&&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) const&> { using type = R(Args...) const&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile> { using type = R(Args...) volatile; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile const> { using type = R(Args...) volatile const; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile&&> { using type = R(Args...) volatile&&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile&> { using type = R(Args...) volatile&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile const&&> { using type = R(Args...) volatile const&&; };
            template<class R, class T, class ...Args>
            struct signature<R(T::*)(Args...) volatile const&> { using type = R(Args...) volatile const&; };
            template<class R, class ...Args>
            struct signature<R(*)(Args...)> { using type = R(Args...); };

            template<class> struct member_function_type;
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...)> { using type = T; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) const> { using type = T const; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...)&&> { using type = T&&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...)&> { using type = T&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) const&&> { using type = T const&&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) const&> { using type = T const&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile> { using type = T volatile; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile const> { using type = T volatile const; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile&&> { using type = T volatile&&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile&> { using type = T volatile&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile const&&> { using type = R(Args...) volatile const&&; };
            template<class R, class T, class ...Args>
            struct member_function_type<R(T::*)(Args...) volatile const&> { using type = R(Args...) volatile const&; };

            template<class> struct function_args;
            template<class R, class ...Args>
            struct function_args<R(Args...)> { using type = std::tuple<Args...>; };
            template<class R, class ...Args>
            struct function_args<R(Args...) const> { using type = std::tuple<Args...>; };
            template<class R, class ...Args>
            struct function_args<R(Args...) volatile> { using type = std::tuple<Args...>; };
            template<class R, class ...Args>
            struct function_args<R(Args...) volatile const> { using type = std::tuple<Args...>; };
            template<class> struct function_return;
            template<class R, class ...Args>
            struct function_return<R(Args...)> { using type = R; };
            template<class R, class ...Args>
            struct function_return<R(Args...) const> { using type = R; };
            template<class R, class ...Args>
            struct function_return<R(Args...) volatile> { using type = R; };
            template<class R, class ...Args>
            struct function_return<R(Args...) volatile const> { using type = R; };

            template<class Ty, class Tuple> struct invocable_tuple;
            template<class Ty, class ...Args>
            struct invocable_tuple<Ty, std::tuple<Args...>>
                : std::bool_constant<std::invocable<Ty, Args...>> {};
        }
        // get the function signature of a (member) function or a lambda/functor
        template<class Ty> using signature_t = typename detail::signature<Ty>::type;
        template<class Ty> using member_function_type_t = typename detail::member_function_type<Ty>::type;
        template<class Ty> using function_args_t = typename detail::function_args<Ty>::type;
        template<class Ty> using function_return_t = typename detail::function_return<Ty>::type;
        template<class Ty, class Tuple> concept invocable_tuple_t = detail::invocable_tuple<Ty, Tuple>::value;

        namespace detail {
            template<class, template<class...> class>
            struct is_specialization : std::false_type {};
            template<template<class...> class Ref, class... Args>
            struct is_specialization<Ref<Args...>, Ref> : std::true_type {};
        }
        // is specialization of templated class
        template<class Test, template<class...> class Ref>
        concept specialization = detail::is_specialization<std::decay_t<Test>, Ref>::value;

        // 
        // Tuple helpers
        // 

        // is specialization of std::tuple
        template<class Ty> concept is_tuple = specialization<Ty, std::tuple>;

        namespace detail {
            template<std::size_t N, is_tuple> struct drop;
            template<class ...Tys> struct drop<0, std::tuple<Tys...>> {
                using type = std::tuple<Tys...>;
            };
            template<std::size_t N, class Ty, class ...Tys> struct drop<N, std::tuple<Ty, Tys...>>
            : drop<N - 1, std::tuple<Tys...>> {};
        }
        // drop first N types from tuple
        template<std::size_t N, is_tuple Ty> using drop_t = typename detail::drop<N, Ty>::type;

        namespace detail {
            template<std::size_t N, is_tuple, is_tuple> struct take;
            template<class ...Tys, class ...Rem> struct take<0, std::tuple<Tys...>, std::tuple<Rem...>> {
                using type = std::tuple<Tys...>;
            };
            template<std::size_t N, class Ty, class ...Tys, class ...Rem> struct take<N, std::tuple<Tys...>, std::tuple<Ty, Rem...>>
            : take<N - 1, std::tuple<Tys..., Ty>, std::tuple<Rem...>> {};
        }
        // take first N types from tuple
        template<std::size_t N, is_tuple Ty> using take_t = typename detail::take<N, std::tuple<>, Ty>::type;

        // remove first type from tuple
        template<is_tuple Ty> using tail_t = drop_t<1, Ty>;
        // first type of tuple
        template<is_tuple Ty> using head_t = std::tuple_element_t<0, Ty>;
        // remove last type from tuple
        template<is_tuple Ty> using init_t = take_t<std::tuple_size_v<Ty> -1, Ty>;
        // last type of tuple
        template<is_tuple Ty> using last_t = std::tuple_element_t<std::tuple_size_v<Ty> -1, Ty>;

        namespace detail {
            template<std::size_t I, class Tuple, std::size_t... Is>
            constexpr auto element_as_tuple(Tuple tuple, std::index_sequence<Is...>) {
                if constexpr (!(std::is_same_v<std::tuple_element_t<I, Tuple>,
                    std::tuple_element_t<Is, Tuple>> || ...))
                    return std::tuple<std::tuple_element_t<I, Tuple>>(std::get<I>(tuple));
                else return std::make_tuple();
            }
            template<class Tuple, std::size_t... Is>
            constexpr auto make_tuple_unique(Tuple tuple, std::index_sequence<Is...>) {
                return std::tuple_cat(element_as_tuple<Is>(tuple, std::make_index_sequence<Is>())...);
            }
        }
        // remove duplicate types from tuple
        template<is_tuple Tuple> constexpr auto unique_tuple(Tuple tuple) {
            return make_tuple_unique(tuple, std::make_index_sequence<std::tuple_size_v<tuple>>{});
        }
        // remove duplicate types from tuples
        template<is_tuple Ty> using unique_tuple_t = decltype(unique_tuple(std::declval<Ty>()));

        namespace detail {
            template<class T, class E, std::size_t I = 0> struct tuple_index;
            template<class F, class ...R, class E, std::size_t I>
            struct tuple_index<std::tuple<F, R...>, E, I> : public std::conditional<std::is_same<E, F>::value,
                std::integral_constant<std::size_t, I>, tuple_index<std::tuple<R...>, E, I + 1>>::type{};
            template<class E, std::size_t I> struct tuple_index<std::tuple<>, E, I> {};
        }
        // index of type in tuple
        template<class E, is_tuple Tuple>
        constexpr static std::size_t tuple_index_v = detail::tuple_index<Tuple, E>::value;

        namespace detail {
            template<class T, is_tuple Ty> struct is_in_tuple;
            template<class T, class ...Tys> struct is_in_tuple<T, std::tuple<Tys...>>
            : std::bool_constant<std::disjunction_v<std::is_same<T, Tys>...>> {};
        }
        // type is in tuple
        template<class T, class Ty> concept in_tuple = is_tuple<Ty> && detail::is_in_tuple<T, Ty>::value;
        // type is not in tuple
        template<class T, class Ty> concept not_in_tuple = is_tuple<Ty> && !detail::is_in_tuple<T, Ty>::value;

        // concat tuples
        template<is_tuple ...Tys> using tuple_cat_t = decltype(std::tuple_cat(std::declval<Tys>()...));

        namespace detail {
            template<class T, is_tuple, is_tuple> struct remove;
            template<class T, class ...Tys> struct remove<T, std::tuple<Tys...>, std::tuple<>> {
                using type = std::tuple<Tys...>;
            };
            template<class T, class Ty, class ...Rem, class ...Tys> struct remove<T, std::tuple<Tys...>, std::tuple<Ty, Rem...>>
            : remove<T, std::tuple<Tys..., Ty>, std::tuple<Rem...>> {};
            template<class T, class ...Rem, class ...Tys> struct remove<T, std::tuple<Tys...>, std::tuple<T, Rem...>>
            : remove<T, std::tuple<Tys...>, std::tuple<Rem...>> {};
        }
        // remove type from tuple
        template<class T, is_tuple Ty> using remove_t = typename detail::remove<T, std::tuple<>, Ty>::type;

        namespace detail {
            struct dud {}; // dud type
            template<class Ty, class T> struct remove_all;
            template<class Ty, class ...E> struct remove_all<Ty, std::tuple<E...>> {
                using type = remove_t<dud, std::tuple<std::conditional_t<in_tuple<E, Ty>, dud, E>...>>; // If in tuple: dud, otherwise type
            };
        }
        // removes all types in tuple from other tuple
        template<is_tuple A, is_tuple B> using remove_all_t = typename detail::remove_all<A, B>::type;

        namespace detail {
            template<class ...Tys> struct as_tuple { using type = std::tuple<Tys...>; };
            template<class Ty> struct as_tuple<Ty> { using type = std::tuple<Ty>; };
            template<class A, class B> struct as_tuple<std::pair<A, B>> { using type = std::tuple<A, B>; };
            template<class ...Tys, template<class...> class T> struct as_tuple<T<Tys...>> { using type = std::tuple<Tys...>; };
            template<class ...Tys> struct as_tuple<std::tuple<Tys...>> { using type = std::tuple<Tys...>; };
        }
        // get type as tuple (pair<a, b> -> tuple<a, b>, type -> tuple<type>, tuple<tys...> -> tuple<tys...>, 
        // type<tys...> -> tuple<tys...>, tys... -> std::tuple<tys...>)
        template<class ...Ty> using as_tuple_t = typename detail::as_tuple<Ty...>::type;

        namespace detail {
            template<is_tuple Ty> struct flatten;
            template<class Ty> struct flatten<std::tuple<Ty>> { using type = std::tuple<Ty>; };
            template<class ...Tys> struct flatten<std::tuple<Tys...>> {
                using type = tuple_cat_t<typename flatten<as_tuple_t<Tys>>::type...>;
            };
        }
        // flatten nested tuples to a single tuple
        template<is_tuple Ty> using flatten_t = typename detail::flatten<Ty>::type;

        namespace detail {
            template<class T, std::size_t ... Is>
            constexpr void print_tuple(auto& a, T& v, std::index_sequence<Is...>) {
                a << "(";
                ((a << std::get<Is>(v) << ", "), ...);
                a << std::get<sizeof...(Is)>(v);
                a << ")";
            }
        }
        // simple tuple printing
        template<class ...Ty>
        constexpr auto& operator<<(auto& a, std::tuple<Ty...>& v) {
            if constexpr (sizeof...(Ty) == 0) a << "()";
            else detail::print_tuple(a, v, std::make_index_sequence<sizeof...(Ty) - 1>{});
            return a;
        }
    }
}