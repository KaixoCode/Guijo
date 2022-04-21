#include "Guijo/Objects/Scrollbar.hpp"

using namespace Guijo;

Scrollbar::Scrollbar(bool vertical) : vertical(vertical) {
	event<Scrollbar>();
	link(fill);

	fill.transition(100.f);
	fill = Color{ 104.f };
	fill[Pressed] = { 161.f };
	fill[Hovering] = { 123.f };
}

void Scrollbar::mouseDrag(const MouseDrag& e) {
	const float length = vertical ? height() : width();
	const float pos = vertical ? (e.pos.y() - e.source.y()) : (e.pos.x() - e.source.x());
	const float norm = pos / (length - barlength());
	scrolled = std::clamp(press + norm * scrolllength(), range[0], range[1]);
}

void Scrollbar::mousePress(const MousePress& e) {
	press = scrolled;
}

bool Scrollbar::hitbox(Point<float> pos) const {
	return bar().contains(pos);
}

void Scrollbar::draw(DrawContext& context) const {
	context.fill(fill);
	context.rect(bar());
}

Dimensions<float> Scrollbar::bar() const {
	if (vertical) return { right() - size, y() + barstart(), size, barlength() };
	else return { x() + barstart(), bottom() - size, barlength(), size };
}

float Scrollbar::scrolllength() const {
	const float rlen = range[1] - range[0];
	return rlen;
}

float Scrollbar::barlength() const {
	const float length = vertical ? height() : width();
	const float rlen = range[1] - range[0] + length;
	const float ratio = length / rlen;
	return std::max(length * ratio, min);
}

float Scrollbar::barstart() const {
	const float length = vertical ? height() : width();
	return (length - barlength()) * (scrolled - range[0]) / (range[1] - range[0]);
}
