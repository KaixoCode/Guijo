#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Animated.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Event/Event.hpp"

namespace Guijo {
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
        constexpr Find match(const StateFind& v) const {
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

    template<class Ty>
    class StateLinked : public StateListener {
    public:
        constexpr StateLinked() {}
        constexpr StateLinked(const Ty& def) : m_Default(def) {}

        constexpr void link(StateId id, const Ty& value) { emplace_or_find(StateLink{ id }) = value; }
        constexpr void link(StateLink link, const Ty& value) { emplace_or_find(link) = value; }

        constexpr Ty& operator[](StateId id) { return emplace_or_find({ id }); }
        constexpr Ty& operator[](StateLink link) { return emplace_or_find(link); }

        virtual StateLinked& operator=(const Ty& val) { m_Default = val; return *this; }
        virtual void value(const Ty& val) { m_Default = val; };

        void update(StateId id, State value) override {
            StateFind _find{ id, value };

            // Remove from state stack if it's a match, so it can
            // more easily be added later without worrying about duplicates
            auto _it = m_StateStack.begin();
            while (_it != m_StateStack.end()) {
                const auto _res = m_Values[*_it].first.match(_find);
                if (_res != StateLink::Find::None) {
                    _it = m_StateStack.erase(_it);
                } else ++_it;
            }

            // Remove current if now out of range
            if (m_Current != npos && m_Values[m_Current].first
                .match(_find) == StateLink::Find::SameId) {
                m_Current = npos;
            }

            bool _addedToStack = false;
            for (std::size_t _i = 0; _i < m_Values.size(); ++_i) {
                const auto _res = m_Values[_i].first.match(_find);
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
                m_Current = *_it;
                m_StateStack.erase(_it);
            }
        };

        const Ty& get() const { 
            return m_Current == npos 
                ? m_Default 
                : m_Values[m_Current].second;
        }

        Ty& get() { 
            return m_Current == npos 
                ? m_Default
                : m_Values[m_Current].second;
        }

        Ty& emplace_or_find(const StateLink& v) {
            for (auto& [a, b] : m_Values) if (a == v) return b;
            return m_Values.emplace_back(std::pair<StateLink, Ty>{ v, {} }).second;
        }

    protected:
        constexpr static std::size_t npos = static_cast<std::size_t>(-1);
        std::size_t m_Current = npos;
        Ty m_Default{};
        std::vector<std::size_t> m_StateStack{};
        std::vector<std::pair<StateLink, Ty>> m_Values{};
    };

    template<class Ty> // Specialization for animated state linked
    class StateLinked<Animated<Ty>> : public StateLinked<Ty>, public Animated<Ty> {
    protected:
        using StateLinked<Ty>::m_Default;
        using StateLinked<Ty>::m_Current;
        using StateLinked<Ty>::m_Values;
        using StateLinked<Ty>::npos;
    public:
        constexpr StateLinked() {}
        constexpr StateLinked(const Animated<Ty>& def) 
            : StateLinked<Ty>(def.goal()), Animated<Ty>{ def } {}

        using StateLinked<Ty>::link;
        using StateLinked<Ty>::operator[];
        using StateLinked<Ty>::emplace_or_find;
        using Animated<Ty>::get;
        using Animated<Ty>::operator Ty;
        using Animated<Ty>::assign;
        using Animated<Ty>::operator=;

        constexpr StateLinked& operator=(const Ty& val) override {
            m_Default = val;
            if (m_Current == npos)
                assign(m_Default);
            return *this;
        }

        constexpr void value(const Ty& val) override {
            m_Default = val;
            if (m_Current == npos)
                assign(m_Default);
        };

        void update(StateId id, State value) override {
            std::size_t _prev = m_Current;
            
            StateLinked<Ty>::update(id, value);

            // Update current value
            if (m_Current != _prev) {
                if (m_Current == npos) assign(m_Default);
                else assign(m_Values[m_Current].second);
            }
        };
    };
}