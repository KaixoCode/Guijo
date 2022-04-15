#include "Guijo/Guijo.hpp"

using namespace Guijo;




class MyObject : public Object {
public:
    Color color{};

    MyObject(Color color) : color(color) {}

    void draw(DrawContext& context) const override {
        if (get(Pressed)) context.fill(color.brighter(0.6));
        else if (get(Hovering)) context.fill(color.brighter(0.8));
        else if (get(Focused)) context.fill(color.brighter(1.2));
        else context.fill(color);

        context.rect(dimensions());
    }
};

constexpr Flex::Box class1{
    .size{ 50.f, 10.f },
    .flex {
        .grow = 1,
    }
};

constexpr Flex::Box class2{
    .size{ 50.f, 10.f },
    .flex {
        .grow = 1,
    }
};

constexpr Flex::Box class3{
    .size{ 50.f, 10.f },
    .flex {
        .grow = 1,
    }
};

int main() {
    constexpr auto aione = sizeof(Object);

    Gui gui;

    Window& window = gui.emplace<Window>({
        .name = "HelloWorld1",
        .dimensions{ -1, -1, 500, 500 },
    });

    window.box.use = false;

    Object& container = window.emplace<Object>();

    container.dimensions({ 0, 0, 500, 500 });

    auto& c1 = container.emplace<MyObject>(Color{ 128, 0, 128 });
    auto& c2 = container.emplace<MyObject>(Color{ 128, 255, 0 });
    auto& c3 = container.emplace<MyObject>(Color{ 255, 0, 128 });

    Size<float> size = { 10, 20 };

    c1.box.size = size;

    c1.box = class1;
    c2.box = class2;
    c3.box = class3;

    while (gui.loop());

    return 0;
}