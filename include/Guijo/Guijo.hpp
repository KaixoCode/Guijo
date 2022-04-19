#pragma once
#include "Guijo/Objects/Object.hpp"
#include "Guijo/Window/Window.hpp"
#include "Guijo/Event/BasicEvents.hpp"
#include "Guijo/Event/StateLinked.hpp"

namespace Guijo {
    class Gui {
    public:
        template<std::derived_from<Window> Ty>
        Pointer<Ty> emplace(const typename Ty::Construct& arg) {
            Ty* _value = new Ty{ arg };
            m_Windows.emplace_back(_value);
            _value->remember();
            return _value;
        }

        bool loop() {
            auto i = m_Windows.begin();
            while (i != m_Windows.end()) {
                if (!(*i)->loop()) m_Windows.erase(i++);
                else ++i;
            }
            return m_Windows.size() != 0;
        }

    private:
        std::list<Pointer<Window>> m_Windows;
    };
}