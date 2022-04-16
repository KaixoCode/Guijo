#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"

namespace Guijo {
    template<class Ty> class RGB;
    template<class Ty> class HSV;
    template<class Ty> class HSL;

    namespace detail {
        template<class Ty>
        constexpr HSV<Ty> rgb2hsv(const RGB<Ty>& rgb) {
            RGB<double> in{ 
                rgb.r() / 255., 
                rgb.g() / 255., 
                rgb.b() / 255., 
                rgb.a() / 255. 
            };
            HSV<double> out{};
            out.a(in.a());

            double min = std::min({ in.r(), in.g(), in.b() });
            double max = std::max({ in.r(), in.g(), in.b() });

            out.v(max);
            double delta = max - min;
            if (delta < 0.00001) return {
                static_cast<Ty>(0),  
                static_cast<Ty>(0),  
                static_cast<Ty>(out.v() * 255.),
                static_cast<Ty>(out.a() * 255.)
            };
            
            if (max <= 0.0) return {
                static_cast<Ty>(NAN),
                static_cast<Ty>(0),
                static_cast<Ty>(out.v() * 255.),
                static_cast<Ty>(out.a() * 255.)
            };

            out.s(delta / max);

            if (in.r() >= max) out.h((in.g() - in.b()) / delta);
            else if (in.g() >= max) out.h(2.0 + (in.b() - in.r()) / delta);
            else out.h(4.0 + (in.r() - in.g()) / delta);
            
            out.h(out.h() * 60.0);
            if (out.h() < 0.0) out.h(out.h() + 360.0);

            return { 
                static_cast<Ty>(255. * out.h() / 360.),
                static_cast<Ty>(255. * out.s()),
                static_cast<Ty>(255. * out.v()),
                static_cast<Ty>(255. * out.a()),
            };
        }

        template<class Ty>
        constexpr RGB<Ty> hsv2rgb(const HSV<Ty>& hsv) {
            HSV<double> in{
                360 * hsv.h() / 255.,
                hsv.s() / 255., 
                hsv.v() / 255., 
                hsv.a() / 255. 
            };
            RGB<double> out{};
            out.a(in.a());

            if (in.s() <= 0.0) return {
                static_cast<Ty>(in.v() * 255.),
                static_cast<Ty>(in.v() * 255.),
                static_cast<Ty>(in.v() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };

            double hh = in.h();
            if (hh >= 360.0) hh = 0.0;
            hh /= 60.0;
            long i = (long)hh;
            double ff = hh - i;
            double p = in.v() * (1.0 - in.s());
            double q = in.v() * (1.0 - (in.s() * ff));
            double t = in.v() * (1.0 - (in.s() * (1.0 - ff)));

            switch (i) {
            case 0: out.r(in.v()), out.g(t), out.b(p); break;
            case 1: out.r(q), out.g(in.v()), out.b(p); break;
            case 2: out.r(p), out.g(in.v()), out.b(t); break;
            case 3: out.r(p), out.g(q), out.b(in.v()); break;
            case 4: out.r(t), out.g(p), out.b(in.v()); break;
            default: out.r(in.v()), out.g(p), out.b(q);break;
            }

            return {
                static_cast<Ty>(out.r() * 255.),
                static_cast<Ty>(out.g() * 255.),
                static_cast<Ty>(out.b() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };
        }

        template<class Ty>
        constexpr HSL<Ty> rgb2hsl(const RGB<Ty>& rgb) {
            RGB<double> in{
                rgb.r() / 255.,
                rgb.g() / 255.,
                rgb.b() / 255.,
                rgb.a() / 255.
            };
            HSL<double> out{};
            out.a(in.a());

            double min = std::min({ in.r(), in.g(), in.b() });
            double max = std::max({ in.r(), in.g(), in.b() });

            out.l((max + min) / 2);

            double delta = max - min;
            if (delta < 0.00001) return {
                static_cast<Ty>(0),
                static_cast<Ty>(0),
                static_cast<Ty>(out.l() * 255.),
                static_cast<Ty>(out.a() * 255.)
            };
            
            if (out.l() <= 0.5) out.s(delta / (max + min));
            else out.s(delta / (2 - max - min));

            if (in.r() >= max) out.h((in.g() - in.b()) / delta);
            else if (in.g() >= max) out.h(2.0 + (in.b() - in.r()) / delta);
            else out.h(4.0 + (in.r() - in.g()) / delta);

            out.h(out.h() * 60.0);
            if (out.h() < 0.0) out.h(out.h() + 360.0);

            return {
                static_cast<Ty>(255. * out.h() / 360.),
                static_cast<Ty>(255. * out.s()),
                static_cast<Ty>(255. * out.l()),
                static_cast<Ty>(255. * out.a()),
            };
        }

        template<class Ty>
        constexpr RGB<Ty> hsl2rgb(const HSL<Ty>& hsl) {
            HSL<double> in{
                hsl.h() / 255.,
                hsl.s() / 255., 
                hsl.l() / 255., 
                hsl.a() / 255. 
            };
            RGB<double> out{};
            out.a(in.a());

            constexpr auto hue2rgb = [](double v1, double v2, double vH) {
                if (vH < 0) vH += 1;
                if (vH > 1) vH -= 1;
                if (6. * vH < 1) return (v1 + (v2 - v1) * 6. * vH);
                if (2. * vH < 1) return v2;
                if (3. * vH < 2) return (v1 + (v2 - v1) * ((2. / 3.) - vH) * 6.);
                return v1;
            };

            if (in.s() <= 0.0) return {
                static_cast<Ty>(in.l() * 255.),
                static_cast<Ty>(in.l() * 255.),
                static_cast<Ty>(in.l() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };
            
            double hue = in.h();
            double v2 = (in.l() < 0.5) ? (in.l() * (1 + in.s())) : ((in.l() + in.s()) - (in.l() * in.s()));
            double v1 = 2 * in.l() - v2;

            out.r(hue2rgb(v1, v2, in.h() + (1. / 3.)));
            out.g(hue2rgb(v1, v2, in.h()));
            out.b(hue2rgb(v1, v2, in.h() - (1. / 3.)));
            
            return {
                static_cast<Ty>(out.r() * 255.),
                static_cast<Ty>(out.g() * 255.),
                static_cast<Ty>(out.b() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };
        }

        template<class Ty>
        constexpr HSV<Ty> hsl2hsv(const HSL<Ty>& hsl) {
            HSL<double> in{
                hsl.h() / 255.,
                hsl.s() / 255.,
                hsl.l() / 255.,
                hsl.a() / 255.
            };
            HSV<double> out{};
            out.a(in.a());
            out.h(in.h());

            out.v(in.s() * std::min(in.l(), 1. - in.l()) + in.l());
            if (out.v() <= 0.) out.s(0);
            else out.s(2. - 2. * in.l() / out.v());

            return {
                static_cast<Ty>(out.h() * 255.),
                static_cast<Ty>(out.s() * 255.),
                static_cast<Ty>(out.v() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };
        }

        template<class Ty>
        constexpr HSL<Ty> hsv2hsl(const HSV<Ty>& hsv) {
            HSV<double> in{
                hsv.h() / 255.,
                hsv.s() / 255.,
                hsv.v() / 255.,
                hsv.a() / 255.
            };
            HSL<double> out{};
            out.a(in.a());
            out.h(in.h());

            out.l(in.v() - in.v() * in.s() / 2.);
            double min = std::min(out.l(), 1. - out.l());
            if (min <= 0) out.s(0);
            else out.s((in.v() - out.l()) / min);

            return {
                static_cast<Ty>(out.h() * 255.),
                static_cast<Ty>(out.s() * 255.),
                static_cast<Ty>(out.l() * 255.),
                static_cast<Ty>(out.a() * 255.),
            };
        }
    }

    template<class Ty>
    class HSV : public VecBase<HSV<Ty>, 4, Ty> { 
        using Parent = VecBase<HSV<Ty>, 4, Ty>;
    public:
        constexpr HSV() : Parent{ {} } {}
        constexpr HSV(const Ty& h, const Ty& s, const Ty& v, const Ty& a) : Parent{ h, s, v, a } {}
        constexpr HSV(const Ty& h, const Ty& s, const Ty& v) : Parent{ h, s, v, 255 } {}
        constexpr HSV(const RGB<Ty>& c) : Parent{ detail::rgb2hsv(c) } {}
        constexpr HSV(const HSL<Ty>& c) : Parent{ detail::hsl2hsv(c) } {}

        constexpr Ty h() const { return this->get<0>(); }
        constexpr Ty s() const { return this->get<1>(); }
        constexpr Ty v() const { return this->get<2>(); }
        constexpr Ty a() const { return this->get<3>(); }

        constexpr operator Vec4<Ty>() const { return { h(), s(), v(), a() }; }
        constexpr RGB<Ty> rgb() const { return *this; }
        constexpr HSL<Ty> hsl() const { return *this; }

        constexpr void h(const Ty& v) { this->get<0>() = v; }
        constexpr void s(const Ty& v) { this->get<1>() = v; }
        constexpr void v(const Ty& v) { this->get<2>() = v; }
        constexpr void a(const Ty& v) { this->get<3>() = v; }
    };

    template<class Ty>
    class HSL : public VecBase<HSL<Ty>, 4, Ty> {
        using Parent = VecBase<HSL<Ty>, 4, Ty>;
    public:
        constexpr HSL() : Parent{ {} } {}
        constexpr HSL(const Ty& h, const Ty& s, const Ty& v, const Ty& a) : Parent{ h, s, v, a } {}
        constexpr HSL(const Ty& h, const Ty& s, const Ty& v) : Parent{ h, s, v, 255 } {}
        constexpr HSL(const RGB<Ty>& c) : Parent{ detail::rgb2hsl(c) } {}
        constexpr HSL(const HSV<Ty>& c) : Parent{ detail::hsv2hsl(c) } {}

        constexpr Ty h() const { return this->get<0>(); }
        constexpr Ty s() const { return this->get<1>(); }
        constexpr Ty l() const { return this->get<2>(); }
        constexpr Ty a() const { return this->get<3>(); }

        constexpr operator Vec4<Ty>() const { return { h(), s(), l(), a() }; }
        constexpr RGB<Ty> rgb() const { return *this; }
        constexpr HSV<Ty> hsv() const { return *this; }

        constexpr void h(const Ty& v) { this->get<0>() = v; }
        constexpr void s(const Ty& v) { this->get<1>() = v; }
        constexpr void l(const Ty& v) { this->get<2>() = v; }
        constexpr void a(const Ty& v) { this->get<3>() = v; }
    };

    template<class Ty>
    class RGB : public VecBase<RGB<Ty>, 4, Ty> {
        using Parent = VecBase<RGB<Ty>, 4, Ty>;
    public:
        constexpr RGB() : Parent{ {} } {}
        constexpr RGB(const Ty& h, const Ty& s, const Ty& v, const Ty& a) : Parent{ h, s, v, a } {}
        constexpr RGB(const Ty& h, const Ty& s, const Ty& v) : Parent{ h, s, v, 255 } {}
        constexpr RGB(const Ty& g, const Ty& a) : Parent{ g, g, g, a } {}
        constexpr RGB(const Ty& g) : Parent{ g, g, g, 255 } {}
        constexpr RGB(const HSV<Ty>& v) { *this = detail::hsv2rgb(v); }
        constexpr RGB(const HSL<Ty>& v) { *this = detail::hsl2rgb(v); }
        constexpr RGB(int hex) : Parent{
            static_cast<Ty>((hex & 0x00FF0000) >> 16),
            static_cast<Ty>((hex & 0x0000FF00) >> 8),
            static_cast<Ty>(hex & 0x000000FF), 
            static_cast<Ty>(255) } {}

        constexpr Ty r() const { return this->get<0>(); }
        constexpr Ty g() const { return this->get<1>(); }
        constexpr Ty b() const { return this->get<2>(); }
        constexpr Ty a() const { return this->get<3>(); }

        constexpr operator Vec4<Ty>() const { return { r(), g(), b(), a() }; }
        constexpr HSV<Ty> hsv() const { return *this; }
        constexpr HSL<Ty> hsl() const { return *this; }

        constexpr void r(const Ty& v) { this->get<0>() = v; }
        constexpr void g(const Ty& v) { this->get<1>() = v; }
        constexpr void b(const Ty& v) { this->get<2>() = v; }
        constexpr void a(const Ty& v) { this->get<3>() = v; }

        constexpr RGB brighter(float percent) const {
            HSL<Ty> _hsv = *this;
            _hsv.l(std::clamp(_hsv.l() * percent, 0.f, 255.f));
            return _hsv;
        }
    };

    using Color = RGB<float>;
}

namespace std {
    template<class Ty>
    struct tuple_size<Guijo::RGB<Ty>> : std::integral_constant<size_t, 4> { };
    template<class Ty, std::size_t N> requires (N < 4)
    struct tuple_element<N, Guijo::RGB<Ty>> { using type = Ty; };
    template<class Ty>
    struct tuple_size<Guijo::HSV<Ty>> : std::integral_constant<size_t, 4> { };
    template<class Ty, std::size_t N> requires (N < 4)
    struct tuple_element<N, Guijo::HSV<Ty>> { using type = Ty; };
    template<class Ty>
    struct tuple_size<Guijo::HSL<Ty>> : std::integral_constant<size_t, 4> { };
    template<class Ty, std::size_t N> requires (N < 4)
    struct tuple_element<N, Guijo::HSL<Ty>> { using type = Ty; };
}