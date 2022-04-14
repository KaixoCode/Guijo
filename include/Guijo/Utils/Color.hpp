#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"

namespace Guijo {

    class Color : public VecBase<Color, 4, float> {
        using Parent = VecBase<Color, 4, float>;
    public:
        constexpr Color() : Parent{ 0, 0, 0, 0, } {}
        constexpr Color(float r, float g, float b, float a) : Parent{ r, g, b, a }  {}
        constexpr Color(float r, float g, float b) : Parent{ r, g, b, 255.f } {}
        constexpr Color(int hex) : Parent{
            static_cast<float>((hex & 0x00FF0000) >> 16),
            static_cast<float>((hex & 0x0000FF00) >> 8),
            static_cast<float>(hex & 0x000000FF), 255.f } {}

        constexpr virtual float r() const { return get<0>(); }
        constexpr virtual float g() const { return get<1>(); }
        constexpr virtual float b() const { return get<2>(); }
        constexpr virtual float a() const { return get<3>(); }
        constexpr virtual void r(float v) { get<0>() = v; }
        constexpr virtual void g(float v) { get<1>() = v; }
        constexpr virtual void b(float v) { get<2>() = v; }
        constexpr virtual void a(float v) { get<3>() = v; }
    };
}

namespace std {
    template<>
    struct tuple_size<Guijo::Color> : std::integral_constant<size_t, 4> { };
    template<std::size_t N> requires (N < 4)
    struct tuple_element<N, Guijo::Color> { using type = float; };
}