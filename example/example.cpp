#include "Guijo/Guijo.hpp"

using namespace Guijo;




class MyObject : public Object {
public:
    Color color{};

    MyObject(Color color) : color(color) {}

    void draw(DrawContext& context) const override {
        if (get(Pressed)) context.fill(color.brighter(0.6));
        else if (get(Hovering)) context.fill(color.brighter(0.8));
        else if (get(Focused)) context.fill(color.brighter(1.5));
        else context.fill(color);

        context.rect(dimensions());
    }
};

constexpr Flex::Box class1{
    .size{ Flex::Value::Auto, 50.f },
    .margin{ 10.f, 10.f, 10.f, 10.f },
    .flex {
        .grow = 1,
        .shrink = 0,
    },
    .align {
        .self = Flex::Align::Stretch
    },
};

constexpr Flex::Box class2{
    .size{ 100.f, 50.f},
    .min{ 50.f, Flex::Value::None },
    .margin{ 10.f, 10.f, 10.f, 10.f },
    .flex {
        .grow = 1,
        .shrink = 1,
    },
    .align {
        .self = Flex::Align::Start
    },
};

constexpr Flex::Box class3{
    .size{ 100.f, 50.f },
    .flex {
        .grow = 1,
        .shrink = 0,
    }
};

int main() {
    constexpr auto aione = sizeof(Object);

    Gui gui;

    Window& window = gui.emplace<Window>({
        .name = "HelloWorld1",
        .dimensions{ -1, -1, 200, 500 },
    });

    window.box.flex.wrap = Flex::Wrap::DoWrap;
    window.box.flex.direction = Flex::Direction::Row;
    window.box.justify = Flex::Justify::Between;
    window.box.align.content = Flex::Align::Center;
    window.box.align.items = Flex::Align::Start;

    Pointer<Object> obj[]{
        window.emplace<MyObject>(Color{ 128,   0, 128 }),
        window.emplace<MyObject>(Color{ 128, 255,   0 }),
        window.emplace<MyObject>(Color{ 128, 255, 128 }),
        window.emplace<MyObject>(Color{ 255,   0,   0 }),
        window.emplace<MyObject>(Color{   0, 255, 128 }),
        window.emplace<MyObject>(Color{   0, 128, 255 }),
    };

    obj[0]->box = class2;
    obj[1]->box = class2;
    obj[2]->box = class1;
    obj[3]->box = class2;
    obj[4]->box = class1;
    obj[5]->box = class1;

    while (gui.loop());

    return 0;
}