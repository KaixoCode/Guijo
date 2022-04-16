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
		else return { Value::Infinite };
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
		Value _value{};
		Value _parent = parent->innerAvailableSize[dim];
		Value _size = size[dim].decode(*this, _parent);
		Value _min = min[dim].decode(*this, _parent);
		Value _max = max[dim].decode(*this, _parent);
		Value _margin1 = margin[dim].decode(*this, _parent);
		Value _margin2 = margin[dim + 2].decode(*this, _parent);

		// If used size is definite, content box has been defined for that dimension, so use that
		if (parent->use && usedSize[dim].definite()) _value = usedSize[dim];
		else if (parent->use && usedSize[dim].definite()) _value = usedSize[dim];
		else if (_size.definite()) _value = _size;
		else if (_parent.definite()) _value = subMargin(_parent, dim, _parent);
		else return Value::Infinite; // Fallback to infinite size

		// Constrain size to min/max if definite
		if (_min.definite()) _value = std::max(_value.value, _min.value);
		if (_max.definite()) _value = std::min(_value.value, _max.value);

		return _value;
	};

	if (parent == nullptr) availableSize = windowSize; // No parent at all: use window size
	else availableSize = { _checkDim(0), _checkDim(1) };

	innerAvailableSize = { 
		subPadding(availableSize[0], 0, parent ? parent->innerAvailableSize[0] : Value::Infinite),
		subPadding(availableSize[1], 0, parent ? parent->innerAvailableSize[1] : Value::Infinite)
	};
}

Value Box::addPadding(Value value, int dim, Value pval) {
	Value _padding1 = padding[dim].decode(*this, pval);
	Value _padding2 = padding[dim + 2].decode(*this, pval);

	// Finally adjust for padding, since we need available size for children
	if (_padding1.definite()) value = value.value + _padding1.value;
	if (_padding2.definite()) value = value.value + _padding2.value;
	return value;
}

Value Box::addMargin(Value value, int dim, Value pval) {
	Value _margin1 = margin[dim].decode(*this, pval);
	Value _margin2 = margin[dim + 2].decode(*this, pval);

	// Finally adjust for padding, since we need available size for children
	if (_margin1.definite()) value = value.value + _margin1.value;
	if (_margin2.definite()) value = value.value + _margin2.value;
	return value;
}
Value Box::subPadding(Value value, int dim, Value pval) {
	Value _padding1 = padding[dim].decode(*this, pval);
	Value _padding2 = padding[dim + 2].decode(*this, pval);

	// Finally adjust for padding, since we need available size for children
	if (_padding1.definite()) value = value.value - _padding1.value;
	if (_padding2.definite()) value = value.value - _padding2.value;
	return value;
}

Value Box::subMargin(Value value, int dim, Value pval) {
	Value _margin1 = margin[dim].decode(*this, pval);
	Value _margin2 = margin[dim + 2].decode(*this, pval);

	// Finally adjust for padding, since we need available size for children
	if (_margin1.definite()) value = value.value - _margin1.value;
	if (_margin2.definite()) value = value.value - _margin2.value;
	return value;
}

Value clamp(Box& self, const Value& pval, Value v, Value min, Value max) {
	auto _v = v.decode(self, pval);
	auto _min = min.decode(self, pval);
	auto _max = max.decode(self, pval);
	if (!_v.definite()) return _v;
	if (_min.definite()) v = std::max(_min.value, v.value);
	if (_max.definite()) v = std::min(_max.value, v.value);
	return v;
}

void Box::format(Object& self) {
	auto& _items = self.objects();

	// Step 1: calculate available size in container
	calcAvailableSize();

	// Parent doesn't exist or doesn't give us a size, use own size
	if (parent == nullptr || !parent->use) {
		if (use) usedSize = availableSize;
		else usedSize = self.size();
	}

	if (!use || _items.size() == 0) { // Don't use flex
		if (parent != nullptr && parent->use) {
			auto _dir = parent->flowDirection() == 1 ? 0 : 1;
			usedSize[_dir] = availableSize[_dir];
			hypoSize[_dir] = availableSize[_dir];
			outerHypoSize[_dir] = addMargin(
				availableSize[_dir], _dir, parent->usedSize[_dir]);
		}
		availableSize = self.size(); // Use its manually set size
		innerAvailableSize = self.size();	
		for (auto& _i : _items) {
			_i->box.parent = this; // No flex parent
			_i->box.format(*_i);
		}
		return;
	}
	
	// Get dimensions
	auto _main = flowDirection();
	auto _cross = _main == 1 ? 0 : 1;

	// Step 2: calculate the preferred size of all the items
	for (auto& _i : _items) {
		auto& _item = _i->box;
		_item.parent = this; // Also update parent
		
		_item.usedSize = { Value::Auto, Value::Auto }; // reset
		
		// Calculate the flex-base-size of this item
		auto _flexBasis = _item.flex.basis.decode(_item, innerAvailableSize[_main]);
		auto _size = _item.size[_main].decode(_item, innerAvailableSize[_main]);
		if (_flexBasis.definite()) _item.flexBaseSize = _flexBasis.value;
		else if (_size.definite()) _item.flexBaseSize = _size.value;
		else _item.flexBaseSize = 0.f; // fallback to 0

		// hypoMainSize is flexBaseSize clamped to min/max values
		_item.hypoSize[_main] = clamp(_item, innerAvailableSize[_main],
			_item.flexBaseSize, _item.min[_main], _item.max[_main]);

		// outerHypoMainSize is hypoMainSize + margin
		_item.outerHypoSize[_main] = _item.hypoSize[_main];
		if (_item.outerHypoSize[_main].definite()) { // If the hypoSize is definite
			_item.outerHypoSize[_main] = _item.addMargin(_item.outerHypoSize[_main], _main, innerAvailableSize[_main]);
		}

		// outerFlexBaseSize is flexBaseSize + margin
		_item.outerFlexBaseSize = _item.flexBaseSize;
		if (_item.outerFlexBaseSize.definite()) { // If the hypoSize is definite
			_item.outerFlexBaseSize = _item.addMargin(_item.outerFlexBaseSize, _main, innerAvailableSize[_main]);
		}
	}

	// Step 3: Main Size Determination
	struct Line {
		std::span<Pointer<Object>> items{};
		float usedSpace = 0;
		float flexGrow = 0;
		float flexShrink = 0;
		float crossSize = 0;
	};
	std::vector<Line> _flexLines;

	// Get available size in flow direction
	auto _availableSize = 0.f; // Since available size is either definite or 
	if (innerAvailableSize[_main].definite()) // infinite, we can do it like this
		_availableSize = innerAvailableSize[_main].value;
	else _availableSize = std::numeric_limits<float>::infinity(); // infinity

	// Single line container, everything in a single line
	if (flex.wrap == Wrap::NoWrap) {
		float _usedSpace = 0;  // used space
		float _flexGrow = 0;   // sum of flex grow 
		float _flexShrink = 0; // sum of flex shrink
		for (auto& _i : _items) {
			auto& _item = _i->box;
			_usedSpace += _item.outerHypoSize[_main].value;
			_flexGrow += _item.flex.grow;
			_flexShrink += _item.flex.shrink;
		}
		_flexLines.push_back({ { _items.begin(), _items.end() },
			_usedSpace, _flexGrow, _flexShrink });
	}
	else {
		// Divide into flex lines
		auto _start = _items.begin(); // Start iterator for line
		float _usedSpace = 0;  // used space
		float _flexGrow = 0;   // sum of flex grow 
		float _flexShrink = 0; // sum of flex shrink
		for (auto _i = _items.begin(); _i != _items.end(); ++_i) {
			auto& _item = (*_i)->box;
			float _thisSize = _item.outerHypoSize[_main].value;
			float _thisGrow = _item.flex.grow;
			float _thisShrink = _item.flex.shrink;
			// Check if going to overflow
			if (_usedSpace + _thisSize > _availableSize) {
				// Create line from start to item (item is end of line, so not included!)
				_flexLines.push_back({ { _start, _i },
					_usedSpace, _flexGrow, _flexShrink });
				_usedSpace = _thisSize;
				_flexGrow = _thisGrow;
				_flexShrink = _thisShrink;
				_start = _i; // Set start of next line
			}
			else { // If no overflow, add to used space and grow/shrink
				_usedSpace += _thisSize;
				_flexGrow += _thisGrow;
				_flexShrink += _thisShrink;
			}
		}
		// If not all have been added to lines, add rest to final line
		if (_start != _items.end())
			_flexLines.push_back({ { _start, _items.end() },
				_usedSpace, _flexGrow, _flexShrink });
	}

	// Resolve flexible lengths (flex grow/shrink)
	for (auto& _line : _flexLines) {
		// size is bigger than available: flex-shrink
		enum class Type { Shrink, Grow }; using enum Type;
		Type _type = _line.usedSpace > _availableSize ? Shrink : Grow;
		auto _flexFactor = _type == Shrink ? &decltype(flex)::shrink : &decltype(flex)::grow;

		// Find used outer flex space
		float _usedOuterFlexSpace = 0;
		for (auto& _item : _line.items) {
			_item->box.freezeSize = false; // reset freeze

			// Freeze items that don't flex, their space usage is target-main-size
			bool _sizing = _type == Shrink // depending on flex type, check less or greater
				? _item->box.flexBaseSize.value < _item->box.hypoSize[_main].value
				: _item->box.flexBaseSize.value > _item->box.hypoSize[_main].value;
			if (_item->box.flex.*_flexFactor == 0 || _sizing) {
				_item->box.targetSize[_main] = _item->box.hypoSize[_main];
				_item->box.freezeSize = true; // No flexing, so freeze size
				_usedOuterFlexSpace += _item->box.addMargin(_item->box.targetSize[_main], _main, _availableSize);
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
					_sumFlexFactor += _item->box.flex.*_flexFactor; // Sum flex factor
					_usedOuterFlexSpace += _item->box.outerFlexBaseSize.value;
				} else {
					_usedOuterFlexSpace += _item->box.addMargin(_item->box.targetSize[_main], _main, _availableSize);
				}
			}
			if (!_itemsLeft) break; // No more left, done

			// Calculate the remaining free space
			float _remainingFreeSpace = _availableSize - _usedOuterFlexSpace;
			if (_sumFlexFactor < 1.f && _initialFreeSpace * _sumFlexFactor < _remainingFreeSpace) 
				_remainingFreeSpace = _initialFreeSpace * _sumFlexFactor;

			// Distribute free space proportional to the flex factor
			if (_remainingFreeSpace != 0) {
				float _sumFlexFactor = 0;
				for (auto& _item : _line.items) {
					if (_item->box.freezeSize) continue; // ignore frozen
					if (_type == Shrink) _sumFlexFactor += _item->box.flex.shrink * _item->box.flexBaseSize.value;
					else _sumFlexFactor += _item->box.flex.grow;
				}

				for (auto& _item : _line.items) {
					if (_item->box.freezeSize) continue; // ignore frozen
					if (_type == Shrink) {
						float _scaledFlexFactor = _item->box.flex.shrink * _item->box.flexBaseSize.value;
						float _ratioFlexFactor = _scaledFlexFactor / _sumFlexFactor;
						_item->box.targetSize[_main] = _item->box.flexBaseSize.value -
							(std::abs(_remainingFreeSpace) * _ratioFlexFactor);
					} else {
						float _ratioFlexFactor = _item->box.flex.grow / _sumFlexFactor;
						_item->box.targetSize[_main] = _item->box.flexBaseSize.value +
							_remainingFreeSpace * _ratioFlexFactor;
					}
				}
			}

			// Fix min/max violations
			float _totalViolation = 0;
			for (auto& _item : _line.items) {
				if (_item->box.freezeSize) continue; // ignore frozen
				// test for min-violation
				if (_item->box.min[_main].definite()) {
					auto _min = _item->box.min[_main].decode(_item->box, _availableSize);
					if (_item->box.targetSize[_main].value < _min.value) {
						_totalViolation += _min.value - _item->box.targetSize[_main].value;
						_item->box.violationType = false;
						_item->box.targetSize[_main].value = _min.value;
					}
				} else { // if not min-constrained, floor at 0 (no negative sizes)
					if (_item->box.targetSize[_main].value < 0) {
						_totalViolation -= _item->box.targetSize[_main].value;
						_item->box.violationType = false;
						_item->box.targetSize[_main].value = 0;
					}
				}
				// test for max-violation
				if (_item->box.max[_main].definite()) {
					auto _max = _item->box.max[_main].decode(_item->box, _availableSize);
					if (_item->box.targetSize[_main].value > _max.value) {
						_totalViolation += _max.value - _item->box.targetSize[_main].value;
						_item->box.violationType = true;
						_item->box.targetSize[_main].value = _max.value;
					}
				}
			}

			for (auto& _item : _line.items) {
				if (_item->box.freezeSize) continue; // ignore frozen
				// Freeze items according to violations
				if (_totalViolation == 0 // No violations
					|| _totalViolation > 0 && _item->box.violationType == false // min-violation
				    || _totalViolation < 0 && _item->box.violationType == true) // max-violation
					_item->box.freezeSize = true;
			}
		}
	}

	// Step 4: Cross Size Determination
	// In order for us to be able to calculate the cross size
	// we need to know the layouts of the items
	for (auto& _item : _items) {
		// Set usedSize in main axis to target size, in recurse, this
		// usedSize will be used for available space
		_item->box.usedSize[_main] = _item->box.targetSize[_main].value;
		_item->box.format(*_item); // TODO: cross size strategies (sometimes not necessary to recurse all the way)
		// After format recurse, the item has chosen a usedSize, set that as
		// hypothetical cross size, as we'll adjust that if align-self: stretch
		_item->box.hypoSize[_cross] = _item->box.usedSize[_cross];
		_item->box.outerHypoSize[_cross] = _item->box.addMargin(
			_item->box.usedSize[_cross], _cross, innerAvailableSize[_cross]);
	}

	auto _crossSize = size[_cross].decode(*this, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] });
	auto _innerCrossSize = _crossSize; 
	if (_crossSize.definite()) {
		// Remove padding from the definite crossSize of the container
		_innerCrossSize = subPadding(_innerCrossSize, 
			_cross, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] });
	}
	if (_flexLines.size() == 1 && _innerCrossSize.definite()) {
		_flexLines[0].crossSize = _crossSize.value; 
	} else {
		for (auto& _line : _flexLines) {
			float _largest = 0;
			for (auto& _item : _line.items) {
				if (_item->box.outerHypoSize[_cross].value > _largest)
					_largest = _item->box.outerHypoSize[_cross].value;
			}
			_line.crossSize = _largest;
		}
	}

	if (_flexLines.size() == 1) { // If only 1 line, clamp it to the min/max
		auto _min = min[_cross].decode(*this, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] });
		auto _max = max[_cross].decode(*this, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] });
		if (_min.definite()) _flexLines[0].crossSize = std::max(_flexLines[0].crossSize, _min.value);
		if (_max.definite()) _flexLines[0].crossSize = std::min(_flexLines[0].crossSize, _max.value);
	}

	// Handle align-content: stretch
	if (align.content == Align::Stretch && _innerCrossSize.definite()) {
		float _sumCrossSize = 0; // Find sum of cross sizes
		for (auto& _line : _flexLines) _sumCrossSize += _line.crossSize;
		if (_sumCrossSize < _innerCrossSize.value) { // If space leftover, add equal portion to all lines
			float _addToCrossSize = (_innerCrossSize.value - _sumCrossSize) / _flexLines.size();
			for (auto& _line : _flexLines) _line.crossSize += _addToCrossSize;
		}
	}

	float _usedCrossSpace = 0;
	for (auto& _line : _flexLines) {
		_usedCrossSpace += _line.crossSize;
		for (auto& _item : _line.items) {
			auto _selfAlign = _item->box.align.self; // Find self align
			if (_item->box.align.self.is(Value::Auto)) // if auto, use container's item align
				_selfAlign = align.items;
			if (!_selfAlign.definite()) _selfAlign = Align::Stretch; // fallback

			if (_selfAlign == Align::Stretch && // If stretch, set to line size - margin
				_item->box.size[_cross].is(Value::Auto)) { 
				_item->box.usedSize[_cross] = _line.crossSize
					- _item->box.margin[_cross].value
					- _item->box.margin[_cross + 2].value;
				_item->box.format(*_item); // Format again with new size
			} else { // No stretch, just use hypo size
				_item->box.usedSize[_cross] = _item->box.hypoSize[_cross].value;
			}
		}
	}

	// Determine container's used cross size
	auto _calcCrossSize = size[_cross].decode(*this, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] });
	if (_calcCrossSize.definite()) usedSize[_cross] = _calcCrossSize.value;
	else usedSize[_cross] = _usedCrossSpace;
	// Clamp to min/max
	usedSize[_cross] = clamp(*this, parent ? parent->innerAvailableSize[_cross] : Value{ windowSize[_cross] },
		usedSize[_cross], min[_cross], max[_cross]);
	
	float _freeCrossSpace = usedSize[_cross] - _usedCrossSpace;
	if (_freeCrossSpace < 0) _freeCrossSpace = 0;

	// Determine content box using padding and usedSize/position
	Vec4<float> _padding{
		padding[0].decode(*this, parent ? parent->innerAvailableSize[0] : Value{ windowSize[0] }),
		padding[1].decode(*this, parent ? parent->innerAvailableSize[1] : Value{ windowSize[1] }),
		padding[2].decode(*this, parent ? parent->innerAvailableSize[0] : Value{ windowSize[0] }),
		padding[3].decode(*this, parent ? parent->innerAvailableSize[1] : Value{ windowSize[1] }),
	};
	
	Dimensions<float> _contentBox;
	_contentBox[_cross] = self[_cross] + _padding[_cross];
	_contentBox[_main] = self[_main] + _padding[_main];
	_contentBox[_cross + 2] = usedSize[_cross] + _padding[_cross + 2];
	_contentBox[_main + 2] = usedSize[_main] + _padding[_main + 2];

	// handle align-content
	Align _content = Align::Start;
	if (align.content.is(Value::Enum))
		_content = static_cast<Align>(align.content.value);

	float _crossStart = 0, _crossDistance = 0, _crossDir = 1;
	switch (_content) {
	case Align::Start:
		_crossStart = _contentBox[_cross];
		_crossDistance = 0;
		_crossDir = 1;
		break;
	case Align::End:
		_crossStart = _contentBox[_cross] + _contentBox[_cross + 2];
		_crossDistance = 0;
		_crossDir = -1;
		break;
	case Align::Center:
		_crossStart = _contentBox.center()[_cross] - _usedCrossSpace / 2.f;
		_crossDistance = 0;
		_crossDir = 1;
		break;
	case Align::Between:
		_crossStart = _contentBox[_cross];
		if (_flexLines.size() == 1) _crossDistance = 0;
		else _crossDistance = std::max(0.f, _freeCrossSpace / (_flexLines.size() - 1));
		_crossDir = 1;
		break;
	case Align::Around:
		_crossStart = _contentBox[_cross] + (_freeCrossSpace / _flexLines.size()) / 2.;
		_crossDistance = _freeCrossSpace / _flexLines.size();
		_crossDir = 1;
		break;
	case Align::Evenly:
		_crossStart = _contentBox[_cross] + _freeCrossSpace / (_flexLines.size() + 1);
		_crossDistance = _freeCrossSpace / (_flexLines.size() + 1);
		_crossDir = 1;
		break;
	}

	Direction _direction = Direction::Row;
	if (flex.direction.is(Value::Enum))
		_direction = static_cast<Direction>(flex.direction.value);

	for (auto& _line : _flexLines) {
		if (_crossDir == -1) _crossStart -= _line.crossSize;

		Justify _justify = Justify::Start;
		if (justify.is(Value::Enum))
			_justify = static_cast<Justify>(justify.value);

		Direction _direction = Direction::Row;
		if (flex.direction.is(Value::Enum))
			_direction = static_cast<Direction>(flex.direction.value);

		float _usedSpace = 0;
		for (auto& _item : _line.items)
			_usedSpace += _item->box.addMargin(_item->box.usedSize[_main],
				_main, innerAvailableSize[_main]);
		
		float _freeSpace = _availableSize - _usedSpace;
		if (_freeSpace < 0) _freeSpace = 0;

		float _start = 0, _distance = 0, _dir = 1;
		switch (_justify) {
		case Justify::Start: 
			_start = _contentBox[_main];
			_distance = 0;
			_dir = 1;
			break;
		case Justify::End: 
			_start = _contentBox[_main] + _contentBox[_main + 2];
			_distance = 0;
			_dir = -1; 
			break;
		case Justify::Center: 
			_start = _contentBox.center()[_main] - _usedSpace / 2.f; 
			_distance = 0;  
			_dir = 1; 
			break;
		case Justify::Between:
			_start = _contentBox[_main];
			if (_line.items.size() == 1) _distance = 0;
			else _distance = std::max(0.f, _freeSpace / (_line.items.size() - 1));
			_dir = 1;
			break;
		case Justify::Around:
			_start = _contentBox[_main] + (_freeSpace / _line.items.size()) / 2.;
			_distance = _freeSpace / _line.items.size();
			_dir = 1;
			break;
		case Justify::Evenly:
			_start = _contentBox[_main] + _freeSpace / (_line.items.size() + 1);
			_distance = _freeSpace / (_line.items.size() + 1);
			_dir = 1;
			break;
		}

		if (_direction == Direction::RowReverse || _direction == Direction::ColumnReverse)
			_dir *= -1, _start = _contentBox.center()[_main] + (_contentBox.center()[_main] - _start);

		for (auto& _item : _line.items) {
			Align _selfAlign = Align::Stretch;
			if (_item->box.align.self.is(Value::Enum))
				_selfAlign = static_cast<Align>(_item->box.align.self.value); // Find self align
			else if (_item->box.align.self.is(Value::Auto)) // if auto, use container's item align
				_selfAlign = static_cast<Align>(align.items.value);

			Vec4<float> _margin{
				_item->box.margin[0].decode(_item->box, innerAvailableSize[0]),
				_item->box.margin[1].decode(_item->box, innerAvailableSize[1]),
				_item->box.margin[2].decode(_item->box, innerAvailableSize[0]),
				_item->box.margin[3].decode(_item->box, innerAvailableSize[1]),
			};

			float _crossOffset = 0;
			switch (_selfAlign) {
			case Align::Start:
				_crossOffset = 0;
				break;		
			case Align::End:
				_crossOffset = _line.crossSize - _item->box.usedSize[_cross];
				break;
			case Align::Center:
				_crossOffset = _line.crossSize / 2. - _item->box.usedSize[_cross] / 2. - _margin[_cross];
				break;
			case Align::Stretch:
				_crossOffset = 0;
				break;
			}

			if (_dir == -1) _start -= (_item->box.usedSize[_main] + _margin[_main] + _margin[_main + 2]);
			(*_item)[_cross] = _crossStart + _margin[_cross] + _crossOffset;
			(*_item)[_main] = _start + _margin[_main];
			(*_item)[_cross + 2] = _item->box.usedSize[_cross];
			(*_item)[_main + 2] = _item->box.usedSize[_main];
			if (_dir == 1) _start += (_item->box.usedSize[_main] + _margin[_main] + _margin[_main + 2]);
			_start += _dir * _distance;
		}

		if (_crossDir == 1) _crossStart += _line.crossSize;
		_crossStart += _crossDir * _crossDistance;
	}
}

