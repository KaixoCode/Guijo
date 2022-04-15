#include "Guijo/Objects/Flex.hpp"
#include "Guijo/Objects/Object.hpp"

using namespace Guijo;
using namespace Flex;

Value Value::decode(Box& obj, Value pval) {
	switch (type) {
	// Normal/Pixels is just the value
	case Normal:
	case Pixels: return value;
	// Percent uses parent size
	case Percent: // If pval is percent or indefinite, we don't know...
		if (!pval.is(Percent) && pval.definite()) return pval.value * value / 100.;
		else return pval;
	// View width and height are percent of window size
	case ViewWidth: return obj.windowSize.width() * value / 100.;
	case ViewHeight: return obj.windowSize.height() * value / 100.;
	// Remain at current value if we can't decode
	default: return *this;
	}
}


int Box::flowDirection() {
	return flex.direction == Direction::Column
		|| flex.direction == Direction::ColumnReverse;
}

void Box::calcAvailableSize() {
	auto _checkDim = [this](int dim) -> Value {
		// If parent is flex-container, and dimensions is flow direction
		// at this point we already know the usedMainSize of this item
		if (parent->use && dim == parent->flowDirection()) return usedMainSize;
		// Otherwise try parentSize/definite size
		auto& _parentSize = parent->availableSize[dim];
		Value _value{};
		Value _size = size[dim].decode(*this, _parentSize);
		Value _min = min[dim].decode(*this, _parentSize);
		Value _max = max[dim].decode(*this, _parentSize);

		if (_size.definite()) _value = _size;
		else if (_parentSize.definite()) {
			Value _padding = padding[dim].decode(*this, _parentSize);
			_value = _parentSize;
			if (_padding.definite()) _value = _value.value - _padding.value;
		} else return Value::Infinite;

		Value _margin = margin[dim].decode(*this, _parentSize);
		if (_margin.definite()) _value = _value.value - _margin.value;

		if (_min.definite() && _max.definite()) // Try constraints
			_value = std::clamp(_value.value, _min.value, _max.value);
		else if (_min.definite()) _value = std::max(_value.value, _min.value);
		else if (_max.definite()) _value = std::min(_value.value, _max.value);
		
		return _value;
	};
	if (parent == nullptr) availableSize = windowSize; // No parent at all: use window size
	else availableSize = { _checkDim(0), _checkDim(1) };
	
}

void Box::calcPreferredSize() {
	auto _dim = parent->flowDirection();
	auto& _parentSize = parent->availableSize[_dim];
	if (flex.basis.definite()) flexBaseSize = flex.basis.decode(*this, _parentSize);
	else if (size[_dim].definite()) flexBaseSize = size[_dim].decode(*this, _parentSize);
	else flexBaseSize = 0.f;
	// hypoMainSize is flexBaseSize clamped to min/max values
	hypoMainSize = flexBaseSize;
	if (min[_dim].definite()) hypoMainSize = std::max(min[_dim].decode(*this, _parentSize).value, hypoMainSize.value);
	if (max[_dim].definite()) hypoMainSize = std::min(max[_dim].decode(*this, _parentSize).value, hypoMainSize.value);
	// outerHypoMainSize is hypoMainSize + margin
	outerHypoMainSize = hypoMainSize;
	if (margin[_dim].definite()) outerHypoMainSize.value += margin[_dim].decode(*this, _parentSize).value;
	if (margin[_dim * 2].definite()) outerHypoMainSize.value += margin[_dim * 2].decode(*this, _parentSize).value;
	// outerFlexBaseSize is flexBaseSize + margin
	outerFlexBaseSize = flexBaseSize;
	if (margin[_dim].definite()) outerFlexBaseSize.value += margin[_dim].decode(*this, _parentSize).value;
	if (margin[_dim * 2].definite()) outerFlexBaseSize.value += margin[_dim * 2].decode(*this, _parentSize).value;
}

void Box::format(Object& self) {
	auto& _container = self.box;
	auto& _items = self.objects();

	if (!_container.use) { // Don't use flex
		_container.availableSize = self.size(); // Use its manually set size
		for (auto& _c : _items) {
			_c->box.parent = &_container; // No flex parent
			_c->box.format(*_c);
		}
		return;
	}

	auto _dim = _container.flowDirection();

	// Step 1: calculate available size in container
	_container.calcAvailableSize();

	// Step 2: calculate the preferred size of all the items
	for (auto& c : _items) {
		c->box.parent = &_container; // Also update parent
		c->box.calcPreferredSize();
	}

	// Step 3: Main Size Determination
	struct Line {
		std::span<Pointer<Object>> items{};
		float usedOuterMainSpace = 0;
		float flexGrow = 0;
		float flexShrink = 0;
		float crossSize = 0;
	};
	std::vector<Line> _flexLines;

	// Get available size in flow direction
	auto _availableSize = 0.f;
	if (_container.availableSize[_dim].definite())
		_availableSize = _container.availableSize[_dim].value;
	else _availableSize = std::numeric_limits<float>::infinity(); // infinity

	// Single line container, everything in a single line
	if (_container.flex.wrap == Wrap::NoWrap) {
		float _usedOuterMainSpace = 0;  // used space
		float _flexGrow = 0;   // sum of flex grow 
		float _flexShrink = 0; // sum of flex shrink
		for (auto& _item : _items) {
			_usedOuterMainSpace += _item->box.outerHypoMainSize.value;
			_flexGrow += _item->box.flex.grow;
			_flexShrink += _item->box.flex.shrink;
		}
		_flexLines.push_back({ { _items.begin(), _items.end() }, 
			_usedOuterMainSpace, _flexGrow, _flexShrink });
	} else {
		// Divide into flex lines
		auto _start = _items.begin(); // Start iterator for line
		float _usedOuterMainSpace = 0;  // used space
		float _flexGrow = 0;   // sum of flex grow 
		float _flexShrink = 0; // sum of flex shrink
		for (auto _i = _items.begin(); _i != _items.end(); ++_i) {
			auto& _item = *_i;
			float _thisOuterMainSize = _item->box.outerHypoMainSize.value;
			float _thisGrow = _item->box.flex.grow;
			float _thisShrink = _item->box.flex.shrink;
			// Check if going to overflow
			if (_usedOuterMainSpace + _thisOuterMainSize > _availableSize) { 
				// Create line from start to item (item is end of line, so not included!)
				_flexLines.push_back({ { _start, _i },  
					_usedOuterMainSpace, _flexGrow, _flexShrink }); 
				_usedOuterMainSpace = _thisOuterMainSize;
				_flexGrow = _thisGrow;
				_flexShrink = _thisShrink;
				_start = _i; // Set start of next line
			} else { // If no overflow, add to used space and grow/shrink
				_usedOuterMainSpace += _thisOuterMainSize;
				_flexGrow += _thisGrow;
				_flexShrink += _thisShrink;
			}
		}
		// If not all have been added to lines, add rest to final line
		if (_start != _items.end())
			_flexLines.push_back({ { _start, _items.end() }, 
				_usedOuterMainSpace, _flexGrow, _flexShrink });
	}

	// Resolve flexible lengths (flex grow/shrink)
	for (auto& _line : _flexLines) {
		// size is bigger than available: flex-shrink
		constexpr bool SHRINK = true;
		constexpr bool GROW = false;
		bool _type = _line.usedOuterMainSpace > _availableSize;
		auto _access = _type ? &decltype(flex)::shrink : &decltype(flex)::grow;

		// Find used outer flex space
		float _usedOuterFlexSpace = 0;
		for (auto& _item : _line.items) {
			_item->box.freezeSize = false; // reset freeze
			// Freeze items that don't flex, their space usage is target-main-size
			bool _sizing = _type == SHRINK // depending on flex type, check less or greater
				? _item->box.flexBaseSize.value < _item->box.hypoMainSize.value
				: _item->box.flexBaseSize.value > _item->box.hypoMainSize.value;
			if (_item->box.flex.*_access == 0 || _sizing) {
				_item->box.targetMainSize = _item->box.hypoMainSize;
				_usedOuterFlexSpace += _item->box.targetMainSize.value;
				_item->box.freezeSize = true; // No shrinking, so freeze size
			} else {
				_usedOuterFlexSpace += _item->box.outerFlexBaseSize.value;
			}
		}
		float _initialFreeSpace = _availableSize - _usedOuterFlexSpace;
		std::size_t _safetyCheck = 100; // Safety check, only allow finite amount of
		while (--_safetyCheck) {		// loops to prevent freezing.
			// Check if there are flexible items left
			bool _itemsLeft = false;
			float _sumFlexFactor = 0;
			_usedOuterFlexSpace = 0;
			for (auto& _item : _line.items) {
				if (!_item->box.freezeSize) {
					_itemsLeft = true; // Found a non-frozen item
					_sumFlexFactor += _item->box.flex.*_access; // Sum flex factor
					_usedOuterFlexSpace += _item->box.outerFlexBaseSize.value;
				} else {
					_usedOuterFlexSpace += _item->box.targetMainSize.value;
				}
			}
			if (!_itemsLeft) break; // No more left, done

			// Calculate the remaining free space
			float _remainingFreeSpace = _availableSize - _usedOuterFlexSpace;
			if (_sumFlexFactor < 1.f && _initialFreeSpace * _sumFlexFactor < _remainingFreeSpace) 
				_remainingFreeSpace = _initialFreeSpace * _sumFlexFactor;

			// Distribute free space proportional to the flex factor
			if (_remainingFreeSpace != 0) {
				// Find scaled flex shrink factors of all items
				if (_type == SHRINK) {
					float _sumScaledFlexFactor = 0;
					for (auto& _item : _items) {
						if (_item->box.freezeSize) continue; // ignore frozen
						_sumScaledFlexFactor += _item->box.flex.shrink * _item->box.flexBaseSize.value;
					}

					for (auto& _item : _items) {
						if (_item->box.freezeSize) continue; // ignore frozen
						float _scaledFlexFactor = _item->box.flex.shrink * _item->box.flexBaseSize.value;
						float _ratioFlexFactor = _scaledFlexFactor / _sumScaledFlexFactor;
						_item->box.targetMainSize = _item->box.flexBaseSize.value -
							(std::abs(_remainingFreeSpace) * _ratioFlexFactor);
					}
				} else {
					float _sumFlexFactor = 0;
					for (auto& _item : _items) {
						if (_item->box.freezeSize) continue; // ignore frozen
						_sumFlexFactor += _item->box.flex.grow;
					}

					for (auto& _item : _items) {
						if (_item->box.freezeSize) continue; // ignore frozen
						float _ratioFlexFactor = _item->box.flex.grow / _sumFlexFactor;
						_item->box.targetMainSize = _item->box.flexBaseSize.value +
							_remainingFreeSpace * _ratioFlexFactor;
					}
				}
			}

			// Fix min/max violations
			float _totalViolation = 0;
			for (auto& _item : _items) {
				if (_item->box.freezeSize) continue; // ignore frozen
				// test for min-violation
				if (_item->box.min[_dim].definite()) {
					auto _min = _item->box.min[_dim].decode(_item->box, _availableSize);
					if (_item->box.targetMainSize.value < _min.value) {
						_totalViolation += _min.value - _item->box.targetMainSize.value;
						_item->box.violationType = false;
						_item->box.targetMainSize.value = _min.value;
					}
				} else { // if not min-constrained, floor at 0 (no negative sizes)
					if (_item->box.targetMainSize.value < 0) {
						_totalViolation -= _item->box.targetMainSize.value;
						_item->box.violationType = false;
						_item->box.targetMainSize.value = 0;
					}
				}
				// test for max-violation
				if (_item->box.max[_dim].definite()) {
					auto _max = _item->box.max[_dim].decode(_item->box, _availableSize);
					if (_item->box.targetMainSize.value > _max.value) {
						_totalViolation += _max.value - _item->box.targetMainSize.value;
						_item->box.violationType = true;
						_item->box.targetMainSize.value = _max.value;
					}
				}
			}

			for (auto& _item : _items) {
				if (_item->box.freezeSize) continue; // ignore frozen
				// Freeze items according to violations
				if (_totalViolation == 0 // No violations
					|| _totalViolation > 0 && _item->box.violationType == false // min-violation
				    || _totalViolation < 0 && _item->box.violationType == true) // max-violation
					_item->box.freezeSize = true;
			}
		}

		for (auto& _item : _items) { // Finally set all usedMainSizes
			_item->box.usedMainSize = _item->box.targetMainSize.value;
		}
	}

	// Step 4: Cross Size Determination
	// In order for us to be able to calculate the cross size
	// we need to know the layouts of the items
	for (auto& _item : _items) {
		_item->box.format(*_item); // TODO: cross size strategies (sometimes not necessary to recurse all the way)
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

