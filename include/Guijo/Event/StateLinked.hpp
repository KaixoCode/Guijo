#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Event/Event.hpp"

namespace Guijo {
    template<class Ty>
    class StateLinked : public StateListener {
        struct StateFind {
            StateId id;
            State value{ 1 };
        };
        struct StateLink {
            enum class Find {
                None, Match, SameId
            };
            StateId id;
            Vec2<State> range{ 1, 1 };
            constexpr Find find(const StateFind& v) const {
                const bool _testId = id == v.id;
                const bool _testRange = range[0] <= v.value && range[1] >= v.value;

                if (_testId && _testRange) return Find::Match;
                if (_testId) return Find::SameId;
                return Find::None;
            }

            constexpr bool operator==(const StateLink& v) const {
                return id == v.id && range == v.range;
            }
        };
    public:
        constexpr StateLinked() {}
        constexpr StateLinked(const Ty& def) : m_Default(def) {}

        constexpr void value(const Ty& def) { m_Default = def; };
        constexpr void link(StateId id, const Ty& value) { m_Values[StateLink{ id }] = value; }
        constexpr void link(StateLink link, const Ty& value) { emplace_or_find(link) = value; }

        constexpr Ty& operator[](StateId id) { return emplace_or_find({ id }); }
        constexpr Ty& operator[](StateLink link) { return emplace_or_find(link); }

        void update(StateId id, State value) override {
            StateFind _find{ id, value };
            std::size_t _prev = m_Current;

            // Remove from state stack if it's a match, so it can
            // more easily be added later without worrying about duplicates
            auto _it = m_StateStack.begin();
            while (_it != m_StateStack.end()) {
                const auto _res = m_Values[*_it].first.find(_find);
                if (_res != StateLink::Find::None) {
                    _it = m_StateStack.erase(_it);
                }
                else ++_it;
            }

            // Remove current if now out of range
            if (m_Current != npos && m_Values[m_Current].first
                .find(_find) == StateLink::Find::SameId) {
                m_Current = npos;
            }

            bool _addedToStack = false;
            for (std::size_t _i = 0; _i < m_Values.size(); ++_i) {
                const auto _res = m_Values[_i].first.find(_find);
                if (_res == StateLink::Find::Match) { // Found a match
                    // No current? set as current!
                    if (m_Current == npos) m_Current = _i;
                    // Current has higher precedence? Add to stack
                    else if (m_Current < _i) {
                        if (!_addedToStack) { // If not already added
                            m_StateStack.push_back(_i);
                            _addedToStack = true;
                        }
                    } 
                    // Current has lower precedence? Add current 
                    // to stack and set new as current
                    else {
                        m_StateStack.push_back(m_Current);
                        m_Current = _i;
                    }
                }
            }

            if (m_Current == npos && !m_StateStack.empty()) {
                auto _it = std::min_element(m_StateStack.begin(), m_StateStack.end());
                m_Current = std::distance(m_StateStack.begin(), _it);
                m_StateStack.erase(_it);
            }

            // Update current value
            if (m_Current != _prev) {
                if (m_Current == npos) m_CurrentValue = m_Default;
                else m_CurrentValue = m_Values[m_Current].second;
            }
        };

        const Ty& get() const { return m_CurrentValue; }
        Ty& get() { return m_CurrentValue; }

        Ty& emplace_or_find(const StateLink& v) {
            for (auto& [a, b] : m_Values) if (a == v) return b;
            return m_Values.emplace_back(std::pair<StateLink, Ty>{ v, {} }).second;
        }

    private:
        constexpr static std::size_t npos = static_cast<std::size_t>(-1);
        std::size_t m_Current = npos;
        Ty m_Default{};
        Ty m_CurrentValue = m_Default;
        std::vector<std::size_t> m_StateStack{};
        std::vector<std::pair<StateLink, Ty>> m_Values{};
    };

}