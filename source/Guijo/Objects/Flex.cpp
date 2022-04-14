#include "Guijo/Objects/Flex.hpp"
#include "Guijo/Objects/Object.hpp"

using namespace Guijo;
using namespace Flex;

float Unit::decode(Box& obj, bool axis) {
	switch (type) {
	// Pixels is just the value
	case Pixels: return value;
	// Percent uses parent size
	case Percent: return obj.parentSize[axis] * value / 100.;
	// View width and height are percent of window size
	case ViewWidth: return obj.windowSize.width() * value / 100.;
	case ViewHeight: return obj.windowSize.height() * value / 100.;
	// Default to Auto if no valid value
	default: return Values::Auto;
	}
}

void Box::refreshPreferredSize() {
	auto sizeInDir = [this](int axis) -> float {
		// If there's a concrete flex-basis value defined, use that
		if (flex.basis.definite()) return flex.basis.decode(*this, axis);
		// If size is indefinite, we'll set preferred to 0
		if (!size[axis].definite()) return 0;
		// Otherwise take defined size
		auto _res = size[axis].decode(*this, axis);
		// clamp to min and max size
		if (min[axis].definite()) _res = std::max(min[axis].decode(*this, axis), _res);
		if (max[axis].definite()) _res = std::min(max[axis].decode(*this, axis), _res);
		// add margin
		if (margin[axis].definite()) _res += margin[axis].decode(*this, axis);
		if (margin[axis*2].definite()) _res += margin[axis*2].decode(*this, axis);
		return _res;
	};
	// Get preferred size in both axes
	preferredSize = { sizeInDir(0), sizeInDir(1) };
}


void Box::format(Object& self) {
	
	auto& container = self.box;
	container.innerSize = self.size();
	container.innerSize[0] -= container.padding[0].decode(container, 0);
	container.innerSize[0] -= container.padding[2].decode(container, 0);
	container.innerSize[1] -= container.padding[1].decode(container, 1);
	container.innerSize[1] -= container.padding[3].decode(container, 1);

	for (auto& c : self.objects()) {
		auto& item = c->box;

		item.parentSize = container.innerSize;
		item.refreshPreferredSize();
	}

	// Single line container
	if (container.flex.wrap == Wrap::NoWrap) {

	}


	/*
		
		preferred size: {
			if (flex-basis is auto) {
				if (size is not auto) {
					if (min-width is not none) return max(width, min-width);
					else return width;
				} else return max-content;
 			} else {
				return flex-basis;
			}
		}

		determine all children's prefered size // Min width needs to be taken into account here!
	
		if enough space in flex direction {
			set all children's prefered size.
			
			check size remaining in flex direction.

			determine sum of children's flex-grow
			if (sum is not 0) {
				determine size of children with a max-size
				for each child that reaches its max size, redivide leftover space
				taking into account the flex-grow proportions
			}

			check justify-content to align items in inline direction when leftover space

			use align-content and align-items to align in block direction

		} else {
			if (nowrap) {
				give all children their prefered size

				check overflow in flex direction.

				determine sum of children's flex-shrink
				if (sum is not 0) {
					determine size of children with a min-size
					for each child that reaches its min size, redistribute space we take
					taking into account the flex-shrink proportions
				} 

				check justify-content to align items in inline direction (perhaps with overflow)

				use align-content and align-items to align in block direction
			} else if (wrap) {
				give all children their prefered size

				start new blocks whenever necessary

				now take every row individually and apply flex-grow/flex-shrink where necessary
				taking into account justify-content when positioning/aligning

				use align-content and align-items to align in block direction
			} else if (wrap-reverse) {
				same as wrap, except the blocks are in reverse order
			}
		}
	
	*/


}

