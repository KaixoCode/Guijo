#include "Guijo/Guijo.hpp"

using namespace Guijo;

class MyObject : public Object {
public:
    int val;
    MyObject(int val) : val(val) {
        event<MyObject>();
    }

    void mouseDrag(const MouseDrag& e) {
        int index = (e.pos.y() / 500) * 6;
        p[index] = e.pos.x();
        std::cout << val << ": Mouse Dragged! (" << e.pos.x() << ", " << e.pos.y() << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mousePress(const MousePress& e) {
        std::cout << val << ": Mouse Pressed! (" << e.pos.x() << ", " << e.pos.y() << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }
    
    void mouseClick(const MouseClick& e) {
        std::cout << val << ": Mouse Clicked! (" << e.pos.x() << ", " << e.pos.y() << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mouseRelease(const MouseRelease& e) {
        std::cout << val << ": Mouse Released! (" << e.pos.x() << ", " << e.pos.y() << ") [" << Key::ToString(e.mod | 'a') << "]\n";
    }

    void mouseWheel(const MouseWheel& e) {
        std::cout << val << ": Mouse Wheel! (" << e.pos.x() << ", " << e.pos.y() << ") " << e.amount << " [" << Key::ToString(e.mod | 'a') << "]\n";
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

    float p[6]{ 0, 0, 0, 0, 0 };
    void draw(DrawContext& context) const override {
        float c = get(Hovering) * 50;
        context.fill({ 255, 255, 255, c });
        context.stroke({ 255, 255, 255, 255 });
        context.strokeWeight(p[0] / 10);
        //context.rect(dimensions().inset(100, 150), { p[1], p[2], p[3], p[4] }, p[5]);
        context.circle(center(), 100.f, { p[1] / 100.f, p[2] / 100.f });
        context.fill({ 255, 255, 255 });
        context.font(Font::Default);
        context.textAlign(Align::Center);
        context.text("test", center());

        context.fill({ 255, 255, 255, 255 });
        for (int i = 0; i < 6; i++) {
            float yp = std::floor(i * height() / 6.);
            context.line({ 0, yp }, { 10, yp });
        }
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

    auto& c1 = container.emplace<MyObject>(0);

    c1.dimensions({ 0, 0, 500, 500 });

    while (gui.loop());

    return 0;
}