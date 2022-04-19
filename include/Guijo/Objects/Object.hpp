#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Objects/Flex.hpp"
#include "Guijo/Objects/EventReceiver.hpp"
#include "Guijo/Objects/Scrollbar.hpp"

namespace Guijo {
    class Object : public EventReceiver {
    public:
        Flex::Box box;

        struct {
            Pointer<Scrollbar> x = new Scrollbar{ false };
            Pointer<Scrollbar> y = new Scrollbar{ true };
            Scrollbar& operator[](std::size_t i) { return i ? *y : *x; }
        } scrollbar;

        Object();
        virtual ~Object() = default;

        virtual bool hitbox(Point<float> pos) const override;
        virtual void handle(const Event& e) override;

        virtual void pre(DrawContext& context) const;
        virtual void draw(DrawContext& context) const;
        virtual void post(DrawContext& context) const;

        virtual void update();

        virtual std::vector<Pointer<Object>>& objects() { return m_Objects; };
        virtual std::vector<Pointer<Object>> const& objects() const { return m_Objects; };

        template<std::derived_from<Object> Ty, class ...Args>
        Pointer<Ty> emplace(Args&&...args);

        template<std::derived_from<Object> Ty>
        void push(Pointer<Ty> object);

        template<auto Fun> void state(std::size_t state);

    private:
        std::vector<Pointer<StateHandler>> m_StateHandlers{};
        std::vector<Pointer<Object>> m_Objects{};
    };

    template<std::derived_from<Object> Ty, class ...Args>
    Pointer<Ty> Object::emplace(Args&&...args) {
        Ty* _value = new Ty{ std::forward<Args>(args)... };
        objects().push_back(dynamic_cast<Object*>(_value));
        _value->remember();
        return _value;
    }

    template<std::derived_from<Object> Ty>
    void Object::push(Pointer<Ty> object) {
        objects().push_back(object);
    }

    template<auto Fun>
    void Object::state(std::size_t state) {
        m_StateHandlers.push_back(new TypedStateHandler<Fun>{ state });
    }
}
