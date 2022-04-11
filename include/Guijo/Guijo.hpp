#pragma once
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Window/Window.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Gui {
	public:
		template<std::derived_from<Window> Ty>
		Ty& emplace(const typename Ty::Construct& arg) {
			Ty* _value = new Ty{ arg };
			m_Windows.push_back(_value);
			return *_value;
		}

		bool loop() {
			for (auto& i : m_Windows) {
				if (!i->loop()) {
					return false;
				}
			}
		}

	private:
		std::vector<Pointer<Window>> m_Windows;
	};
}