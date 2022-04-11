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

    void mouseMove(const MouseMove& e) {
        // std::cout << val << ": Mouse Moved! (" << e.pos[0] << ", " << e.pos[1] << ")\n";
    }

    void mouseDrag(const MouseDrag& e) {
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

    void draw(DrawContext& context) const override {
        //context.rect({ dimensions() });
    }
};

int main() {
    Gui gui;

    Pointer<Window> window = gui.emplace<Window>({ 
        .name = "HelloWorld",
        .dimensions{ -1, -1, 500, 500 },
    });

    Pointer<Object> container = window->emplace<Object>();

    container->dimensions({ 0, 0, 500, 500 });
    container->set(MyState);

    auto& c1 = container->emplace<MyObject>(0);
    auto& c2 = container->emplace<MyObject>(1);
    auto& c3 = container->emplace<MyObject>(2);
    auto& c4 = container->emplace<MyObject>(3);

    c1.dimensions({   0,   0, 250, 250 });
    c4.dimensions({   0, 250, 250, 250 });
    c3.dimensions({ 250,   0, 250, 250 });
    c2.dimensions({ 250, 250, 250, 250 });

    while (gui.loop());

    return 0;
}