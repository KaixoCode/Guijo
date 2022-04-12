#include "Guijo/Guijo.hpp"

using namespace Guijo;

const std::size_t MyState = Object::newState();
const std::size_t MyOtherState = Object::newState();

// Custom event
struct MyEvent : Event {
    MyEvent(std::size_t count) : count(count) {}
    std::size_t count{ 1 };
    bool forward(const Object& obj) const override { return obj.get(MyState); }
};

class MyObject : public Object {
public:
    int val;
    MyObject(int val) : val(val) {
        event<MyObject>();
    }



    void mouseDrag(const MouseDrag& e) {
        r = e.pos[0];
        std::cout << val << ": Mouse Dragged! (" << e.pos[0] << ", " << e.pos[1] << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mousePress(const MousePress& e) {
        std::cout << val << ": Mouse Pressed! (" << e.pos[0] << ", " << e.pos[1] << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }
    
    void mouseClick(const MouseClick& e) {
        std::cout << val << ": Mouse Clicked! (" << e.pos[0] << ", " << e.pos[1] << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mouseRelease(const MouseRelease& e) {
        std::cout << val << ": Mouse Released! (" << e.pos[0] << ", " << e.pos[1] << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mouseWheel(const MouseWheel& e) {
        std::cout << val << ": Mouse Wheel! (" << e.pos[0] << ", " << e.pos[1] << ") " << e.amount << " [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void focus(const Focus& e) {
        std::cout << val << ": Focus!\n";
    }

    void unfocus(const Unfocus& e) {
        std::cout << val << ": Unfocus!\n";
    }

    void mouseEnter(const MouseEnter& e) {
        std::cout << val << ": Enter!\n";
    }

    void mouseExit(const MouseExit& e) {
        std::cout << val << ": Exit!\n";
    }
    float r = 0;
    void draw(DrawContext& context) const override {
        float c = get(Hovering) * 100 + 100;
        context.fill({ Color{ val == 0 || val == 3 ? c : 0, val == 1 || val == 3 ? c : 0, val == 2 ? c : 0, 255.f } });
        context.rect({ .rect = dimensions(), .radius = 50.f + r });
    }
};

int main() {
    Gui gui;

    Window& window = gui.emplace<Window>({ 
        .name = "HelloWorld",
        .dimensions{ -1, -1, 500, 500 },
    });

    Object& container = window.emplace<Object>();

    container.dimensions({ 0, 0, 500, 500 });
    container.set(MyState);

    auto& c1 = container.emplace<MyObject>(0);
    auto& c2 = container.emplace<MyObject>(1);
    auto& c3 = container.emplace<MyObject>(2);
    auto& c4 = container.emplace<MyObject>(3);

    c1.dimensions({  10,  10, 400, 250 });
    c4.dimensions({  30, 100, 250, 250 });
    c3.dimensions({ 200,  20, 250, 350 });
    c2.dimensions({ 250, 200, 200, 250 });

    while (gui.loop());

    return 0;
}