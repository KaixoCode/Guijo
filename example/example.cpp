#include "Guijo/pch.hpp"
#include "Guijo/Object.hpp"
#include "Guijo/BasicEvents.hpp"

using namespace Guijo;

const std::size_t MyState = Object::NewState();
const std::size_t MyOtherState = Object::NewState();

// Custom event
struct MyEvent : Event {
    MyEvent(std::size_t count) : count(count) {}
    std::size_t count{ 1 };
    bool Forward(const Object& obj) const override { return obj.has(MyState); }
};

class MyObject : public Object {
public:
    MyObject() {
        // Register event handler through member function
        HandleEvent<&MyObject::MouseMove>();
        // Register event handler through lambda, with self parameter
        HandleEvent<[](MyObject& self, const MyEvent&) {
            std::cout << "MyEvent!\n";
        }>();
        // Register event handler through lambda, without self parameter
        HandleEvent<[](const ::MyEvent& e) {}>();
        // Register state handler with match count
        HandleState<[](const ::MyEvent& e, Object& o, std::size_t matches) {
            return matches < e.count&& o.has(Visible); // Matches first 'count' visible sub-objects
        }>(MyOtherState);
        // Register state handler without match count
        HandleState<[](const MousePress& e, Object& o) {
            return o.Hitbox(e.pos); // Matches all objects that were clicked on
        }>(MyState);
    }

    void MouseMove(const MouseMove& e) {
        std::cout << "Mouse Moved!\n";
    }

    void Draw(DrawContext& context) override {}
};

int main() {
    Pointer<MyObject> container = new MyObject;

    auto& c1 = container->emplace<Object>();
    auto& c2 = container->emplace<Object>();
    auto& c3 = container->emplace<Object>();
    auto& c4 = container->emplace<Object>();

    c1.dimensions({ 0, 0, 10, 10 });
    c2.dimensions({ 2, 2, 10, 10 });
    c3.dimensions({ 4, 4, 10, 10 });
    c4.dimensions({ 6, 6, 10, 10 });

    container->Handle(MyEvent{ 2 });

    auto a1 = c1.has(MyOtherState); // True
    auto a2 = c2.has(MyOtherState); // True
    auto a3 = c3.has(MyOtherState); // False
    auto a4 = c4.has(MyOtherState); // False

    c1.set(Visible, false);

    container->Handle(MyEvent{ 1 });

    auto d1 = c1.has(MyOtherState); // False
    auto d2 = c2.has(MyOtherState); // True
    auto d3 = c3.has(MyOtherState); // False
    auto d4 = c4.has(MyOtherState); // False

    container->Handle(MousePress{ { 3, 3 } });

    auto b1 = c1.has(MyState); // True
    auto b2 = c2.has(MyState); // True
    auto b3 = c3.has(MyState); // False
    auto b4 = c4.has(MyState); // False

    container->Handle(MyEvent{ 2 });

    container->Handle(MouseMove{ { 7, 7 } });
    return 0;
}