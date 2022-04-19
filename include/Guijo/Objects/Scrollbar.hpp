#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Objects/EventReceiver.hpp"
#include "Guijo/Utils/Pointer.hpp"
#include "Guijo/Event/Event.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Scrollbar : public EventReceiver {
	public:
		Scrollbar(bool vertical) : vertical(vertical) {
			event<Scrollbar>();
			link(fill);

			fill = Color{ 104.f };
			fill[Pressed] = { 161.f };
			fill[Hovering] = { 123.f };
		}

		void mouseDrag(const MouseDrag& e) {
			scrolled = press + e.pos.x() - e.source.x();
			scrolled = std::clamp(scrolled, range[0], range[1]);
		}

		void mousePress(const MousePress& e) {
			press = scrolled;
		}

		bool hitbox(Point<float> pos) const override {
			if (vertical) return contains(pos) && pos.x() > right() - size;
			else return contains(pos) && pos.y() > bottom() - size;
		}

		void draw(DrawContext& context) const {
			context.fill(fill);
			if (vertical) context.rect({ right() - size, y(), size, height() }, 0);
			else context.rect({ x(), bottom() - size, width(), size }, 0);
		}

		bool visible = false;
		bool vertical = false;
		Vec2<float> range{ 0, 0 };
		float scrolled = 0;
		float size = 15;
		StateLinked<Animated<Color>> fill;

	private:
		float press = 0;
	};
}