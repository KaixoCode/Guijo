#include "Guijo/Guijo.hpp"

using namespace Guijo;

/*
TODO:
 - custom logger
*/


class MyObject : public Object, public StateListener {
public:
    Color color;

    StateLinked<Animated<float>> border{ 
        { 0.f, 500., Curves::easeOut<2>, }
    };

    void update(StateId id, State value) {
        std::cout << id << ": " << value << '\n';
    }

    MyObject(Color color) : color(color) {

        link(*this);
        link(border); // register StateLinked value

        border[Hovering] = 20.f;
        border[Focused] = 10.f;
    }

    void draw(DrawContext& context) const override {
        context.fill(color);
        context.stroke(color.brighter(1.5));
        context.strokeWeight(border);
        context.rect(dimensions());
        Object::draw(context);
    }
};

auto class1 = []() {
    Flex::Class _class;
    _class.margin = 5.f;
    _class.margin.transition(200.f);
    _class.margin.curve(Curves::easeOut<2>);
    _class.margin[Hovering] = 15.f;

    _class.size.transition(200.);
    _class.size.curve(Curves::easeOut<2>);
    _class.size.width[Hovering] = 180.f;
    return _class;
}();

int main() {
    constexpr auto aione = sizeof(Object);

    StateLinked<float> a{ 2 };
    a[Hovering] = 10;

    a.update(Hovering, 1);

    auto& val1 = a.get();

    a.update(Hovering, 0);

    auto& val2 = a.get();


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

    for (auto& o : obj) {
        auto& _item = o->box;
        _item.size = { 200.f, Flex::Value::Auto },
        _item.margin = Vec4<float>{ 5.f, 5.f, 5.f, 5.f };
        _item.padding = Vec4<float>{ 5.f, 5.f, 5.f, 5.f };
        _item.flex = {
            .grow = 1,
            .shrink = 1,
            .wrap = Flex::DoWrap,
        };
        _item.justify = Flex::Justify::Start;
        _item.align = {
            .content = Flex::Align::Stretch,
            .items = Flex::Align::Stretch,
            .self = Flex::Align::Stretch,
        };
        _item = class1;
    }

    while (gui.loop());

    return 0;
}