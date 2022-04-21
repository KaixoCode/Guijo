#include "Guijo/Guijo.hpp"

using namespace Guijo;

/*
TODO:
 - custom logger
*/

class MyObject : public Object {
public:
    StateLinked<Animated<Color>> color;

    MyObject(Color c) : color{ { c, 0., Curves::easeOut<2> } } {
        link(color);

        color[Focused] = c.brighter(1.6);
        color[Hovering] = c.brighter(1.3);

        box.size.height.transition(0.f);
        box.size.height[Hovering] = 300;
    }

    void draw(DrawContext& context) const override {
        context.fill(color);
        context.rect(dimensions());
        Object::draw(context);
    }
};

struct WindowStyle : Flex::Class {
    WindowStyle() {
        flex.wrap = Flex::Wrap::NoWrap;
        flex.direction = Flex::Direction::Column;
        justify = Flex::Justify::Start;
        align.content = Flex::Align::Stretch;
        align.items = Flex::Align::Stretch;
        padding = 15.f;
    }
};

struct Class1 : Flex::Class {
    Class1() {
        size.width = Flex::Value::Auto;
        size.height = 200;
        margin = 10.f;
        flex.wrap = Flex::Wrap::NoWrap;
        flex.direction = Flex::Direction::Row;
        flex.grow = 0;
        flex.shrink = 0;
        justify = Flex::Justify::Start;
        align.content = Flex::Align::Stretch;
        align.items = Flex::Align::Stretch;
    }
};

struct Class11 : Flex::Class {
    Class11() {
        size.height = Flex::Value::Auto;
        size.width = 30.f;
        margin = 5.f;
        flex.grow = 0;
        flex.shrink = 0;
        align.self = Flex::Value::Auto;
    }
};




class Button : public Object, public StateListener {
public:
    struct Behaviour : public Refcounted {
        virtual void trigger() = 0;
    };

    struct Click : public Behaviour {
        const Click(std::invocable auto const& fun) : fun(fun) {}
        std::function<void()> fun;
        void trigger() override { fun(); }
    };

    struct Toggle : public Behaviour {
        const Toggle(std::invocable<bool> auto const& fun) : fun(fun) {}
        std::function<void(bool)> fun;
        bool state = false;
        void trigger() override { fun(state ^= true); }
    };

    struct Graphics : public Refcounted, public StateListener {
        void update(StateId id, State value) override { for (auto& i : listeners) i->update(id, value); }
        virtual void draw(const Dimensions<float>& dims, DrawContext& context) const = 0;
        template<std::derived_from<StateListener> Ty>
        void link(Ty& val) { listeners.push_back(&val); }
    private:
        std::vector<StateListener*> listeners{};
    };

    struct Default : public Graphics {
        StateLinked<Animated<Color>> fill{};
        StateLinked<Animated<Color>> border{};
        StateLinked<Animated<Color>> color{};
        StateLinked<Animated<float>> borderWidth{};
        float fontSize = 20.f;
        std::string text = "Button";
        std::string font{ Font::Default };

        Default() {
            border = Color{ 95.f };
            borderWidth = 2.f;
            fill.curve(Curves::easeOut<5>);
            fill.transition(200.f);
            fill = Color{ 45.f };
            fill[Pressed] = Color{ 75.f };
            fill[Hovering] = Color{ 55.f };
            color = Color{ 255.f };
            link(fill);
            link(border);
            link(color);
            link(borderWidth);
        }

        void draw(const Dimensions<float>& dims, DrawContext& context) const override {
            context.fill(fill);
            context.stroke(border);
            context.strokeWeight(borderWidth);
            context.rect(dims);
            context.textAlign(Align::Center);
            context.fill(color);
            context.font(font);
            context.fontSize(fontSize);
            context.text(text, dims.center());
        }
    };

    Button() { 
        event<Button>();
        link(*this);
    }

    Button(std::derived_from<Behaviour> auto const& behaviour)
        : behaviour(new std::decay_t<decltype(behaviour)>{ behaviour }) {
        event<Button>();
        link(*this);
    }

    void mouseRelease(const MouseRelease& e) {
        if (hitbox(e.pos)) behaviour->trigger();
    }

    void draw(DrawContext& context) const override {
        graphics->draw(dimensions(), context);
    }

    Pointer<Behaviour> behaviour = new Click{ []() {} };
    Pointer<Graphics> graphics = new Default{};
private:

    void update(StateId id, State value) override {
        if (graphics) graphics->update(id, value);
    }
};

int main() {
    
    Gui gui;

    auto window = gui.emplace<Window>({
        .name = "HelloWorld1",
        .dimensions{ -1, -1, 800, 800 },
    });

    window->box = WindowStyle{};
    window->box.use = true;
    window->box.align.content = Flex::Align::Center;
    window->box.align.items = Flex::Align::Center;
    window->box.justify = Flex::Align::Center;

    auto obj = window->emplace<Button>(Button::Toggle{ [](bool v) { std::cout << "click: " << v << "\n"; } });
    obj->box.use = true;
    obj->box.size = { 400, 50 };

    while (gui.loop());

    return 0;
}