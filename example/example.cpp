#include "Guijo/Guijo.hpp"

using namespace Guijo;




class MyObject : public Object {
public:
    MyObject() {
        event<MyObject>();
    }

    void draw(DrawContext& context) const override {

    }

    void update() override {

    }
};

int main() {


    Flex::Box box;

    MyObject age{};

    age.box = box;

    constexpr Degrees a = -360;
    constexpr Degrees b = -180;
    constexpr auto res = (b + a).normalized();

    Gui gui;

    Window& window = gui.emplace<Window>({
        .name = "HelloWorld1",
        .dimensions{ -1, -1, 500, 500 },
    });

    Object& container = window.emplace<Object>();

    container.dimensions({ 0, 0, 500, 500 });

    auto& c1 = container.emplace<MyObject>();

    c1.dimensions({ 0, 0, 500, 500 });
    
    while (gui.loop());

    return 0;
}