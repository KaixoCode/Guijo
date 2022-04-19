#include "Guijo/Guijo.hpp"

using namespace Guijo;

/*
TODO:
 - custom logger
*/

class MyObject : public Object {
public:
    StateLinked<Animated<Color>> color;

    MyObject(Color c) : color{ { c, 200., Curves::easeOut<2> } } {
        link(color);

        color[Focused] = c.brighter(1.6);
        color[Hovering] = c.brighter(1.3);
    }

    void draw(DrawContext& context) const override {
        context.fill(color);
        context.rect(dimensions());
        Object::draw(context);
    }
};

struct WindowStyle : Flex::Class {
    WindowStyle() {
        flex.wrap = Flex::Wrap::DoWrap;
        flex.direction = Flex::Direction::Row;
        justify = Flex::Justify::Center;
        align.content = Flex::Align::Stretch;
        align.items = Flex::Align::Stretch;
        padding = { 15.f, 15.f, 15.f, 15.f };
    }
};

struct Class1 : Flex::Class {
    Class1() {
        size.width = Flex::pc{ 50.f };
        margin = 10.f;
        flex.wrap = Flex::Wrap::DoWrap;
        flex.direction = Flex::Direction::Row;
        flex.grow = 0;
        flex.shrink = 1;
        justify = Flex::Justify::Evenly;
        align.content = Flex::Align::Stretch;
        align.items = Flex::Align::Stretch;
    }
};

struct Class11 : Flex::Class {
    Class11() {
        min.height = 30.f;
        size.height = Flex::Value::Auto;
        size.width = 30.f;
        margin = 5.f;
        flex.grow = 0;
        flex.shrink = 0;
        align.self = Flex::Value::Auto;
    }
};

int main() {
    Gui gui;

    auto window = gui.emplace<Window>({
        .name = "HelloWorld1",
        .dimensions{ -1, -1, 800, 800 },
    });

    window->box = WindowStyle{};

    //for (int i = 0; i < 10; i++) {
        auto obj = window->emplace<MyObject>(Color{ 20.f });
        obj->box = Class1{};
        for (int i = 0; i < 500; i++) {
            auto o = obj->emplace<MyObject>(Color{ 40.f });
            o->box = Class11{};
        }
    //}
    while (gui.loop());

    return 0;
}