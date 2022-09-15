#include "Guijo/Guijo.hpp"
#include <ranges>

using namespace Guijo;

struct Beacon : Object {
    Point<double> location{ 0, 0 };
    double distance = 100;
    bool enabled = true;

    bool dragging = false;
    bool adjusting = false;

    StateLinked<Animated<Color>> color;

    Beacon() {
        box.use = false;
        link(color);

        location.x(rand() % 500);
        location.y(rand() % 500);

        Color _c = HSV<float>{ static_cast<float>(rand() % 50) * 5, 128.f, 255.f, 100.f };
        color = _c;
        color[Hovering] = _c.brighter(0.8);

        event<Beacon>();
    }

    void mousePress(const MousePress& event) {
        if (event.button & MouseButton::Left) {
            dragging = true;
        } else if (event.button & MouseButton::Right) {
            adjusting = true;
        } else if (event.button & MouseButton::Middle) {
            enabled ^= true;
        }
    }

    void mouseRelease(const MouseRelease& event) {
        dragging = false;
        adjusting = false;
    }

    void mouseDrag(const MouseDrag& event) {
        if (dragging) 
            location = event.pos;
        if (adjusting)
            distance = event.pos.distance(location);
    }

    bool hitbox(Point<float> pos) const override {
        return pos.distance(location) < distance;
    }

    void pre(DrawContext& p) const override {}
    void post(DrawContext& p) const override {}

    void draw(DrawContext& p) const override {
        Object::draw(p);
        if (enabled) {
            p.fill(color);
            p.circle(location, distance);
        } 
        p.fill({ 255, 255, 255, 255 });
        p.circle(location, 3);
    }

    void update() override {
        pos(location);
    }
};

struct Guess : Object {

    Point<double> guess{ 300, 300 };

    void pre(DrawContext& p) const override {}
    void post(DrawContext& p) const override {}
    void draw(DrawContext& p) const override {
        p.fill({ 255, 0, 0, 255 });
        p.circle(guess, 10);
    }
};

struct PointWDist : Point<double> {
    double distance;
};

int main() {
    Gui _gui;

    auto _window = _gui.emplace<Window>({
        .name = "Test",
        .dimensions{ -1, -1, 800, 500 },
    });

    _window->box.use = false;
    _window->box.size.width = Flex::pc{ 100 };
    _window->box.size.height = Flex::pc{ 100 };

    std::vector<Pointer<Beacon>> beacons;

    beacons.push_back(_window->emplace<Beacon>());
    beacons.push_back(_window->emplace<Beacon>());
    beacons.push_back(_window->emplace<Beacon>());
    beacons.push_back(_window->emplace<Beacon>());
    beacons.push_back(_window->emplace<Beacon>());

    auto _guess = _window->emplace<Guess>();

    while (_gui.loop()) {
        using namespace std::views;

        constexpr auto distancePointCircle = [](Point<double> point, Point<double> circle, double radius) {
            return std::abs(std::sqrt(std::pow(point.x() - circle.x(), 2) + std::pow(point.y() - circle.y(), 2)) - radius);
        };

        double sumDistancesAtMouse = 0.0;
        for (auto& beacon : beacons) {
            if (!beacon->enabled) continue;
            sumDistancesAtMouse += distancePointCircle(_window->cursor.position, beacon->location, beacon->distance);
        }

        std::cout << sumDistancesAtMouse << '\n';

        auto filtered = filter(beacons, [](Pointer<Beacon>& beacon) { return beacon->enabled; });
        auto minX = std::ranges::min(transform(filtered, [](Pointer<Beacon>& beacon) { return beacon->location.x(); }));
        auto maxX = std::ranges::max(transform(filtered, [](Pointer<Beacon>& beacon) { return beacon->location.x(); }));
        auto minY = std::ranges::min(transform(filtered, [](Pointer<Beacon>& beacon) { return beacon->location.y(); }));
        auto maxY = std::ranges::max(transform(filtered, [](Pointer<Beacon>& beacon) { return beacon->location.y(); }));
        auto h = std::max(maxX - minX, maxY - minY) * 3.;
        auto center = Point<double>{ minX + (maxX - minX) / 2., minY + (maxY - minY) / 2. };

        std::list<PointWDist> points{};

        auto addAround = [&](Point<double>& p) {
            double x = p.x();
            double y = p.y();
            double offsets[]{ 0.0, h / 3.0, -h / 3.0 };
            for (auto xOffset : offsets) for (auto yOffset : offsets)
                if (!(xOffset == 0.0 && yOffset == 0.0))
                    points.push_back({ { x + xOffset, y + yOffset }, 0.0 });
        };

        points.push_back({ center, 0.0 });

        constexpr std::size_t iterations = 10;

        double smallestFoundDistance = 1e10;
        for (std::size_t i = 0; i < iterations; ++i) {
            std::size_t size = points.size();
            auto begin = points.begin();
            for (std::size_t j = 0; j < size; ++j) {
                addAround(*begin++);
            }

            h /= 3.0;

            for (auto& point : points) {
                double sumOfDistances = 0.0;
                for (auto& beacon : beacons) {
                    if (!beacon->enabled) continue;
                    sumOfDistances += distancePointCircle(point, beacon->location, beacon->distance);
                }

                smallestFoundDistance = std::min(smallestFoundDistance, sumOfDistances);
                point.distance = sumOfDistances;
            }

            for (auto _i = points.begin(); _i != points.end();) {
                auto& point = *_i;
                if (point.distance > smallestFoundDistance + (3 * h) / std::numbers::sqrt2) {
                    _i = points.erase(_i);
                } else ++_i;
            }

            smallestFoundDistance = 1e10;
        }

        _guess->guess = points.front();
    }
}

