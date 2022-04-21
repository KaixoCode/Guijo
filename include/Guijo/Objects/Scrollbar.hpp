#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Objects/EventReceiver.hpp"
#include "Guijo/Utils/Pointer.hpp"
#include "Guijo/Event/Event.hpp"
#include "Guijo/Event/BasicEvents.hpp"

namespace Guijo {
	class Scrollbar : public EventReceiver {
	public:
		Scrollbar(bool vertical);

		void mouseDrag(const MouseDrag& e);
		void mousePress(const MousePress& e);

		bool hitbox(Point<float> pos) const override;
		void draw(DrawContext& context) const;

		bool visible = false;
		bool vertical = false;
		Vec2<float> range{ 0, 0 };
		Animated<float> scrolled{ 0.f, 100.f, Curves::easeOut<2> };
		float size = 15;
		float min = 25;
		StateLinked<Animated<Color>> fill;

	protected:
		float press = 0;

		Dimensions<float> bar() const;
		float scrolllength() const;
		float barlength() const;
		float barstart() const;
	};
}