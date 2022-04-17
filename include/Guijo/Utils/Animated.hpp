#pragma once
#include "Guijo/pch.hpp"

namespace Guijo {

	using Curve = double(*)(double);
	namespace Curves {
		namespace detail {
			constexpr double power(double x, int n) {
				if (n == 0) return 1.;
				if (n % 2 == 0) {
					double m = power(x, n / 2);
					return m * m;
				} else return x * power(x, n - 1);
			}
		}
		constexpr auto linear = [](double v) {
			return v;
		};

		template<std::size_t N> requires (N > 0)
		constexpr auto easeIn = [](double x) {
			return detail::power(x, N);
		};

		template<std::size_t N> requires (N > 0)
		constexpr auto easeOut = [](double x) {
			return 1. - detail::power(1. - x, N);
		};

		template<std::size_t N> requires (N > 0)
		constexpr auto easeInOut = [](double x) {
			const double xp = detail::power(x, N);
			const double pp = detail::power(-2. * x + 2., N);
			const double tp = detail::power(2., N - 1);
			return x < 0.5 ? tp * xp : 1. - pp / 2.;
		};

		constexpr auto easeInOutElastic = [](double x) {
			const double c5 = (2. * std::numbers::pi_v<double>) / 4.5;
			return x == 0. ? 0. : x == 1. ? 1.
				: x < 0.5 ? -(std::pow(2., 20. * x - 10.) * std::sin((20. * x - 11.125) * c5)) / 2.
				: (std::pow(2., -20. * x + 10.) * std::sin((20. * x - 11.125) * c5)) / 2. + 1.;
		};

		constexpr auto easeOutElastic = [](double x) {
			const double c4 = (2. * std::numbers::pi_v<double>) / 3.;
			return x == 0. ? 0. : x == 1. ? 1.
			  : std::pow(2., -10. * x) * std::sin((x * 10. - 0.75) * c4) + 1.;
		};

		constexpr auto easeInElastic = [](double x) {
			const double c4 = (2. * std::numbers::pi_v<double>) / 3.;
			return x == 0. ? 0. : x == 1. ? 1.
				: -std::pow(2., 10. * x - 10.) * std::sin((x * 10. - 10.75) * c4);
		};

		constexpr auto easeOutBounce = [](double x) {
			constexpr double n1 = 7.5625;
			constexpr double d1 = 2.75;

			if (x < 1. / d1) return n1 * x * x;
			else if (x < 2. / d1) return n1 * (x -= 1.5 / d1) * x + 0.75;
			else if (x < 2.5 / d1) return n1 * (x -= 2.25 / d1) * x + 0.9375;
			else return n1 * (x -= 2.625 / d1) * x + 0.984375;
		};

		constexpr auto easeInBounce = [](double x) {
			return 1 - easeOutBounce(1 - x);
		};

		constexpr auto easeInOutBounce = [](double x) {
			return x < 0.5
				? (1 - easeOutBounce(1 - 2 * x)) / 2
				: (1 + easeOutBounce(2 * x - 1)) / 2;
		};
	}

	template<class Ty>
	class Animated {
	public:
		constexpr Animated() = default;
		constexpr Animated(const Animated&) = default;
		constexpr Animated(Animated&&) = default;
		constexpr Animated(double millis) : m_Time(millis) {}
		constexpr Animated(Curve curve) : m_Curve(curve) {}
		constexpr Animated(const Ty& val) : m_Goal(val) {}
		constexpr Animated(const Ty& val, double millis) : m_Goal(val), m_Time(millis) {}
		constexpr Animated(const Ty& val, Curve curve) : m_Goal(val), m_Curve(curve) {}
		constexpr Animated(const Ty& val, double millis, Curve curve) : m_Goal(val), m_Time(millis), m_Curve(curve) {}

		constexpr Animated& operator=(const Animated&) = default;
		constexpr Animated& operator=(Animated &&) = default;
		constexpr Animated& operator=(const Ty& val) { return assign(val); }
		constexpr Animated& assign(const Ty& newval) {
			m_Value = get();
			m_Goal = newval;
			m_ChangeTime = std::chrono::steady_clock::now();
			return *this;
		}

		constexpr void curve(Curve curve) { m_Curve = curve; };
		constexpr void transition(double millis) { m_Time = millis; }
		constexpr void jump(const Ty& val) { m_Goal = val, m_Value = val; }

		constexpr Ty& goal() { return m_Goal; }
		constexpr const Ty& goal() const { return m_Goal; }

		constexpr Ty get() const {
			if (m_Time == 0 || m_Value == m_Goal) return m_Goal;
			const double _percent = std::clamp(
				std::chrono::duration_cast<std::chrono::milliseconds>(
					std::chrono::steady_clock::now() - m_ChangeTime)
					.count() / m_Time, 0., 1.);
			return m_Value + (m_Goal - m_Value) * m_Curve(_percent);
		}

		constexpr operator Ty() const { return get(); }

	protected:
		double m_Time = 0;
		Ty m_Goal{};
		Ty m_Value = m_Goal;
		std::chrono::steady_clock::time_point m_ChangeTime{};
		Curve m_Curve = Curves::linear;
	};
}