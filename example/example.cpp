#include "Guijo/Guijo.hpp"

using namespace Guijo;

/*
TODO:
 - class attribute system perhaps
 - link values to state

*/


class MyObject : public Object {
public:
    Color color{};

    MyObject(Color color) : color(color) {
        box.margin[0].transition = 100;
        box.margin[1].transition = 100;
        box.margin[2].transition = 100;
        box.margin[3].transition = 100;
        box.size[0].transition = 100;
        box.size[1].transition = 100;
    }

    void draw(DrawContext& context) const override {
        if (get(Pressed)) context.fill(color.brighter(0.6f));
        else if (get(Hovering)) context.fill(color.brighter(0.8f));
        else if (get(Focused)) context.fill(color.brighter(1.5f));
        else context.fill(color);

        context.rect(dimensions());
        Object::draw(context);
    }

    bool hovering = false;
    void update() {
        //if (get(Hovering)) {
        //    if (!hovering) {
        //        hovering = true;
        //        box.margin = { 10.f, 10.f, 10.f, 10.f };
        //        box.size = { 50.f, 50.f };
        //    }
        //} else {
        //    if (hovering) {
        //        hovering = false;
        //        box.margin = { 0.f, 0.f, 0.f, 0.f };
        //        box.size = { 70.f, 70.f };
        //    }
        //}
    }
};

constexpr Flex::Box class1{
    .size{ 200.f, Flex::Value::Auto },
    .margin{ 5.f, 5.f, 5.f, 5.f },
    .padding{ 5.f, 5.f, 5.f, 5.f },
    .flex {
        .grow = 1,
        .shrink = 1,
        .wrap = Flex::DoWrap,
    },
    .justify = Flex::Justify::Start,
    .align {
        .content = Flex::Align::Stretch,
        .items = Flex::Align::Stretch,
        .self = Flex::Align::Stretch,
    },
};

constexpr Flex::Box class2{
    .size{ 50.f, Flex::Value::Auto },
    .margin{ 5.f, 5.f, 5.f, 5.f },
    .flex {
        .grow = 0,
        .shrink = 0,
    },
    .align {
        .self = Flex::Align::Stretch
    },
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
    window.box.justify = Flex::Justify::Start;
    window.box.align.content = Flex::Align::Stretch;
    window.box.align.items = Flex::Align::Stretch;
    window.box.padding = { 5.f, 5.f, 5.f, 5.f };

    Pointer<Object> obj[]{
        window.emplace<MyObject>(Color{ 40.f }),
        window.emplace<MyObject>(Color{ 30.f }),
        window.emplace<MyObject>(Color{ 70.f }),
        window.emplace<MyObject>(Color{ 50.f }),
        window.emplace<MyObject>(Color{ 20.f }),
        window.emplace<MyObject>(Color{ 60.f }),
    };

    for (auto& o : obj)
        o->box = class1;

    Pointer<Object> obj2[]{
        obj[0]->emplace<MyObject>(Color{ 240.f }),
        obj[0]->emplace<MyObject>(Color{ 210.f }),
        obj[0]->emplace<MyObject>(Color{ 180.f }),
        obj[1]->emplace<MyObject>(Color{ 150.f }),
        obj[1]->emplace<MyObject>(Color{ 120.f }),
        obj[1]->emplace<MyObject>(Color{  90.f }),
    };

    for (auto& o : obj2)
        o->box = class2;

    while (gui.loop());

    return 0;
}