#pragma once
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Window/Window.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Gui {
		struct WindowHandler {
			WindowHandler(auto ctor) : window(ctor()) {}
			Pointer<Window> window{ nullptr };
		};
	public:
		template<std::derived_from<Window> Ty>
		Ty& emplace(const typename Ty::Construct& arg) {
			WindowHandler& v = m_Windows.emplace_back([&arg]() {
				return new Ty{ arg }; });
			return *dynamic_cast<Ty*>(v.window.get());
		}

		bool loop() {
			auto i = m_Windows.begin();
			while (i != m_Windows.end()) {
				if (!i->window->loop()) m_Windows.erase(i++);
				else ++i;
			}
			return m_Windows.size() != 0;
		}

	private:
		std::list<WindowHandler> m_Windows;
	};
}