#include "Guijo/Objects/Flex.hpp"
#include "Guijo/Objects/Object.hpp"

using namespace Guijo;
using namespace Flex;

Value Value::decode(Box& obj, Value pval) {
    switch (type) {
    // Normal/Pixels is just the value
    case Pixels: return get();
    // Percent uses parent size
    case Percent: // If pval is percent or indefinite, we don't know...
        if (!pval.is(Percent) && pval.definite()) return pval.get() * get() / 100.;
        else return { Value::Infinite };
    // View width and height are percent of window size
    case ViewWidth: return obj.windowSize.width() * get() / 100.;
    case ViewHeight: return obj.windowSize.height() * get() / 100.;
    // Remain at current value if we can't decode
    default: return *this;
    }
}

float Value::current() {
    if (transition == 0) return value;

    const auto _now = std::chrono::steady_clock::now();
    const auto _duration = std::chrono::duration_cast<std::chrono::milliseconds>(_now - m_ChangeTime).count();
    const float _percent = std::clamp(_duration / transition, 0.f, 1.f);

    return pvalue * (1.f - _percent) + value * _percent;
}

void Value::trigger(float newval, Type newtype) {
    pvalue = current();
    value = newval;
    // If change of type, don't transition
    if (newtype != type) 
        type = newtype, pvalue = value;
    m_ChangeTime = std::chrono::steady_clock::now();
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
        else return Value::Infinite; // Fallback to infinite size

        // Constrain size to min/max if definite
        if (_min.definite()) _value = std::max(_value.get(), _min.get());
        if (_max.definite()) _value = std::min(_value.get(), _max.get());

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
    if (_padding1.definite()) value = value + _padding1;
    if (_padding2.definite()) value = value + _padding2;
    return value;
}

Value Box::addMargin(Value value, int dim, Value pval) {
    Value _margin1 = margin[dim].decode(*this, pval);
    Value _margin2 = margin[dim + 2].decode(*this, pval);

    // Finally adjust for padding, since we need available size for children
    if (_margin1.definite()) value = value + _margin1;
    if (_margin2.definite()) value = value + _margin2;
    return value;
}
Value Box::subPadding(Value value, int dim, Value pval) {
    Value _padding1 = padding[dim].decode(*this, pval);
    Value _padding2 = padding[dim + 2].decode(*this, pval);

    // Finally adjust for padding, since we need available size for children
    if (_padding1.definite()) value = value - _padding1;
    if (_padding2.definite()) value = value - _padding2;
    return value;
}

Value Box::subMargin(Value value, int dim, Value pval) {
    Value _margin1 = margin[dim].decode(*this, pval);
    Value _margin2 = margin[dim + 2].decode(*this, pval);

    // Finally adjust for padding, since we need available size for children
    if (_margin1.definite()) value = value - _margin1;
    if (_margin2.definite()) value = value - _margin2;
    return value;
}

Value clamp(Box& self, const Value& pval, Value v, Value min, Value max) {
    auto _v = v.decode(self, pval);
    auto _min = min.decode(self, pval);
    auto _max = max.decode(self, pval);
    if (!_v.definite()) return _v;
    if (_min.definite()) v = std::max(_min.get(), v.get());
    if (_max.definite()) v = std::min(_max.get(), v.get());
    return v;
}

void Box::format(Object& self) {
    auto& _items = self.objects();

    // ===================================================
    // Step 1: calculate available size in container
    // ===================================================

    calcAvailableSize();

    // Parent doesn't exist or doesn't give us a size, use own size
    if (parent == nullptr || !parent->use) {
        if (use) usedSize = availableSize;
        else usedSize = self.size();
    }

    // Don't use flex or no items
    if (!use || _items.size() == 0) { 
        // When we have a parent, we need to set the usedSize in
        // the cross direction of the parent.
        if (parent != nullptr && parent->use) {
            auto _dir = parent->flowDirection() == 1 ? 0 : 1;
            usedSize[_dir] = availableSize[_dir];
        }
        // All objects need these 2 for their children.
        availableSize = self.size(); 
        innerAvailableSize = self.size();
        // recurse to children after updating parent
        for (auto& _i : _items) {
            _i->box.parent = this;
            _i->box.format(*_i);
        }
        return; // and stop here, because no items or not using flex
    }
    
    // Get dimensions
    auto _main = flowDirection();
    auto _cross = _main == 1 ? 0 : 1;
    auto _parentSize = parent ? parent->innerAvailableSize : Size<Value>{ windowSize };

    // ===================================================
    // Step 2: calculate the preferred size of all the items
    // ===================================================

    for (auto& _i : _items) {
        auto& _item = _i->box;
        _item.parent = this; // Also update parent

        // Before starting the algorithm, reset to Auto
        _item.usedSize = { Value::Auto, Value::Auto }; 
        
        // Calculate the flex-base-size of this item
        auto _flexBasis = _item.flex.basis.decode(_item, innerAvailableSize[_main]);
        auto _size = _item.size[_main].decode(_item, innerAvailableSize[_main]);
        if (_flexBasis.definite()) _item.flexBaseSize = _flexBasis;
        else if (_size.definite()) _item.flexBaseSize = _size;
        else _item.flexBaseSize = 0.f; // fallback to 0

        // hypoMainSize is flexBaseSize clamped to min/max values
        _item.hypoSize[_main] = clamp(_item, innerAvailableSize[_main],
            _item.flexBaseSize, _item.min[_main], _item.max[_main]);

        // outerHypoMainSize is hypoMainSize + margin
        _item.outerHypoSize[_main] = _item.hypoSize[_main];
        if (_item.outerHypoSize[_main].definite()) { // If the hypoSize is definite
            _item.outerHypoSize[_main] = _item.addMargin(
                _item.outerHypoSize[_main], _main, innerAvailableSize[_main]);
        }

        // outerFlexBaseSize is flexBaseSize + margin
        _item.outerFlexBaseSize = _item.flexBaseSize;
        if (_item.outerFlexBaseSize.definite()) { // If the hypoSize is definite
            _item.outerFlexBaseSize = _item.addMargin(
                _item.outerFlexBaseSize, _main, innerAvailableSize[_main]);
        }
    }

    // ===================================================
    // Step 3: Collect into lines
    // ===================================================

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
        _availableSize = innerAvailableSize[_main];
    else _availableSize = std::numeric_limits<float>::infinity(); // infinity

    // Single line container, everything in a single line
    if (flex.wrap == Wrap::NoWrap) {
        float _usedSpace = 0;  // used space
        float _flexGrow = 0;   // sum of flex grow 
        float _flexShrink = 0; // sum of flex shrink
        for (auto& _i : _items) {
            auto& _item = _i->box;
            _usedSpace += _item.outerHypoSize[_main];
            _flexGrow += _item.flex.grow;
            _flexShrink += _item.flex.shrink;
        }
        _flexLines.push_back({ { _items.begin(), _items.end() },
            _usedSpace, _flexGrow, _flexShrink });
    } else {
        // Divide into flex lines
        auto _start = _items.begin(); // Start iterator for line
        float _usedSpace = 0;  // used space
        float _flexGrow = 0;   // sum of flex grow 
        float _flexShrink = 0; // sum of flex shrink
        for (auto _i = _items.begin(); _i != _items.end(); ++_i) {
            auto& _item = (*_i)->box;
            float _thisSize = _item.outerHypoSize[_main];
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

    // ===================================================
    // Step 4: Resolve flexible lengths (flex grow/shrink)
    // ===================================================

    for (auto& _line : _flexLines) {
        // Determine if we're growing or shrinking items
        enum class Type { Shrink, Grow }; using enum Type;
        Type _type = _line.usedSpace > _availableSize ? Shrink : Grow;
        auto _flexFactor = _type == Shrink ? &decltype(flex)::shrink : &decltype(flex)::grow;

        // Freeze items that don't flex, their space usage is target-main-size
        for (auto& _i : _line.items) {
            auto& _item = _i->box;
            _item.freezeSize = false; // initial reset
            bool _sizing = _type == Shrink // depending on flex type, check less or greater
                ? _item.flexBaseSize < _item.hypoSize[_main]
                : _item.flexBaseSize > _item.hypoSize[_main];
            if (_item.flex.*_flexFactor == 0 || _sizing) {
                _item.targetSize[_main] = _item.hypoSize[_main];
                _item.freezeSize = true; // No flexing, so freeze size
            }
        }

        // Find the used space in the main axis in this line
        float _usedOuterFlexSpace = 0;
        for (auto& _i : _line.items) {
            auto& _item = _i->box;
            _usedOuterFlexSpace += _item.freezeSize
                ? _item.addMargin(_item.targetSize[_main], _main, _availableSize)
                : _item.outerFlexBaseSize;
        }
        float _initialFreeSpace = _availableSize - _usedOuterFlexSpace;
        
        std::size_t _safetyCheck = 100; // Safety check, only allow finite amount of
        while (--_safetyCheck) {        // loops to prevent freezing.
            // Check if there are flexible items left
            bool _itemsLeft = false;
            for (auto& _i : _line.items) {
                auto& _item = _i->box;
                if (!_item.freezeSize) {
                    _itemsLeft = true;
                    break;
                }
            }
            if (!_itemsLeft) break; // No more left, done

            // Find the sum of all the non-frozen flex-factors, and
            // the used outer flex space (flex-basis + margin)
            float _sumFlexFactor = 0;
            _usedOuterFlexSpace = 0;
            for (auto& _i : _line.items) {
                auto& _item = _i->box;
                if (_item.freezeSize) { // When frozen, use target-size + margin
                    _usedOuterFlexSpace += _item.addMargin(
                        _item.targetSize[_main], _main, _availableSize);
                } else { // Non-frozen: add to flex-factor sum, and use outer-flex-base-size
                    _sumFlexFactor += _item.flex.*_flexFactor;
                    _usedOuterFlexSpace += _item.outerFlexBaseSize;
                }
            }

            // Calculate the remaining free space
            float _remainingFreeSpace = _availableSize - _usedOuterFlexSpace;
            if (_sumFlexFactor < 1.f && _initialFreeSpace * _sumFlexFactor < _remainingFreeSpace) 
                _remainingFreeSpace = _initialFreeSpace * _sumFlexFactor;

            // Distribute remaining free space proportional to the flex factor
            if (_remainingFreeSpace != 0) {
                // Find the sum of the flex factor
                float _sumFlexFactor = 0;
                for (auto& _i : _line.items) {
                    auto& _item = _i->box;
                    if (_item.freezeSize) continue; // ignore frozen
                    _sumFlexFactor += _type == Shrink 
                        ? _item.flex.shrink * _item.flexBaseSize
                        : static_cast<float>(_item.flex.grow);
                }
                // Adjust size of items proportional to their 
                // flex-factor using the flex-factor sum
                for (auto& _i : _line.items) {
                    auto& _item = _i->box;
                    if (_item.freezeSize) continue; // ignore frozen
                    if (_type == Shrink) {
                        float _scaledFlexFactor = _item.flex.shrink * _item.flexBaseSize;
                        float _ratioFlexFactor = _scaledFlexFactor / _sumFlexFactor;
                        _item.targetSize[_main] = _item.flexBaseSize -
                            (std::abs(_remainingFreeSpace) * _ratioFlexFactor);
                    } else {
                        float _ratioFlexFactor = _item.flex.grow / _sumFlexFactor;
                        _item.targetSize[_main] = _item.flexBaseSize +
                            _remainingFreeSpace * _ratioFlexFactor;
                    }
                }
            }

            // Fix min/max violations
            float _totalViolation = 0;
            for (auto& _i : _line.items) {
                auto& _item = _i->box;
                if (_item.freezeSize) continue; // ignore frozen
                // test for min-violation
                if (_item.min[_main].definite()) {
                    auto _min = _item.min[_main].decode(_item, _availableSize);
                    if (_item.targetSize[_main] < _min) {
                        _totalViolation += _min - _item.targetSize[_main];
                        _item.violationType = false;
                        _item.targetSize[_main] = _min;
                    }
                } else { // if not min-constrained, floor at 0 (no negative sizes)
                    if (_item.targetSize[_main] < 0) {
                        _totalViolation -= _item.targetSize[_main];
                        _item.violationType = false;
                        _item.targetSize[_main] = 0;
                    }
                }
                // test for max-violation
                if (_item.max[_main].definite()) {
                    auto _max = _item.max[_main].decode(_item, _availableSize);
                    if (_item.targetSize[_main] > _max) {
                        _totalViolation += _max - _item.targetSize[_main];
                        _item.violationType = true;
                        _item.targetSize[_main] = _max;
                    }
                }
            }

            for (auto& _i : _line.items) {
                auto& _item = _i->box;
                if (_item.freezeSize) continue; // ignore frozen
                // Freeze items according to violations
                if (_totalViolation == 0 // No violations
                    || _totalViolation > 0 && _item.violationType == false // min-violation
                    || _totalViolation < 0 && _item.violationType == true) // max-violation
                    _item.freezeSize = true;
            }
        }
    }

    // ===================================================
    // Step 5: Cross Size Determination
    // ===================================================
    
    // In order for us to be able to calculate the cross size
    // we need to know the layouts of the items
    for (auto& _i : _items) {
        auto& _item = _i->box;
        // Set usedSize in main axis to target size, in recurse, this
        // usedSize will be used for available space
        _item.usedSize[_main] = _item.targetSize[_main];
        _item.format(*_i); 
        // After format recurse, the item has chosen a usedSize, set that as
        // hypothetical cross size, as we'll adjust that if align-self: stretch
        _item.hypoSize[_cross] = _item.usedSize[_cross];
        _item.outerHypoSize[_cross] = _item.addMargin(
            _item.usedSize[_cross], _cross, innerAvailableSize[_cross]);
    }

    // Check container for definite cross-size
    auto _crossSize = size[_cross].decode(*this, _parentSize[_cross]);
    auto _innerCrossSize = _crossSize; 
    if (_crossSize.definite()) {
        // Remove padding from the definite crossSize of the container
        _innerCrossSize = subPadding(_innerCrossSize, _cross, _parentSize[_cross]);
    }

    // Determine the cross sizes of all the lines
    if (_flexLines.size() == 1 && _innerCrossSize.definite()) {
        _flexLines[0].crossSize = _innerCrossSize; // 1 line: use container inner-cross-size
    } else { // Otherwise use largest hypothetical-cross-size of item in line
        for (auto& _line : _flexLines) {
            float _largest = 0;
            for (auto& _i : _line.items) {
                auto& _item = _i->box;
                if (_item.outerHypoSize[_cross] > _largest)
                    _largest = _item.outerHypoSize[_cross];
            }
            _line.crossSize = _largest;
        }
    }

    // If only 1 line, clamp it to the min/max
    if (_flexLines.size() == 1) { 
        auto _min = min[_cross].decode(*this, _parentSize[_cross]);
        auto _max = max[_cross].decode(*this, _parentSize[_cross]);
        if (_min.definite()) _flexLines[0].crossSize = std::max(_flexLines[0].crossSize, static_cast<float>(_min));
        if (_max.definite()) _flexLines[0].crossSize = std::min(_flexLines[0].crossSize, static_cast<float>(_max));
    }

    // Handle align-content: stretch, divide leftover space to lines equaly
    if (align.content == Align::Stretch && _innerCrossSize.definite()) {
        float _sumCrossSize = 0; // Find sum of cross sizes
        for (auto& _line : _flexLines) _sumCrossSize += _line.crossSize;
        if (_sumCrossSize < _innerCrossSize) { // If space leftover, add equal portion to all lines
            float _addToCrossSize = (_innerCrossSize - _sumCrossSize) / _flexLines.size();
            for (auto& _line : _flexLines) _line.crossSize += _addToCrossSize;
        }
    }

    // Find the used-cross-size of all the items
    float _usedCrossSpace = 0; // While keeping track of it
    for (auto& _line : _flexLines) {
        _usedCrossSpace += _line.crossSize;
        for (auto& _item : _line.items) {
            auto _selfAlign = _item->box.align.self; // Find self align
            if (_item->box.align.self.is(Value::Auto)) // if auto, use container's item align
                _selfAlign = align.items;
            if (!_selfAlign.definite()) _selfAlign = Align::Stretch; // fallback
            // If stretch, set used-cross-size to line's cross-size minus margin
            if (_selfAlign == Align::Stretch && 
                _item->box.size[_cross].is(Value::Auto)) { 
                _item->box.usedSize[_cross] = _line.crossSize
                    - _item->box.margin[_cross]
                    - _item->box.margin[_cross + 2];
                _item->box.format(*_item); // Format again with new size
            } else { // No stretch, just use hypothetical-cross-size
                _item->box.usedSize[_cross] = _item->box.hypoSize[_cross];
            }
        }
    }

    // Determine container's used cross size
    auto _calcCrossSize = size[_cross].decode(*this, _parentSize[_cross]);
    if (_calcCrossSize.definite()) usedSize[_cross] = _calcCrossSize;
    else usedSize[_cross] = _usedCrossSpace;
    // Clamp to min/max
    usedSize[_cross] = clamp(*this, _parentSize[_cross], 
        usedSize[_cross], min[_cross], max[_cross]);
    
    // ===================================================
    // Step 6: Axis-Alignment
    // ===================================================

    // Determine if we have free space in the cross dimension
    float _freeCrossSpace = usedSize[_cross] - _usedCrossSpace;
    if (_freeCrossSpace < 0) _freeCrossSpace = 0;

    // Determine content box using padding and usedSize/position
    Vec4<float> _padding{
        padding[0].decode(*this, _parentSize[0]),
        padding[1].decode(*this, _parentSize[1]),
        padding[2].decode(*this, _parentSize[0]),
        padding[3].decode(*this, _parentSize[1]),
    };
    
    // Determine the content box, so we know where to position items
    Dimensions<float> _contentBox;
    _contentBox[_cross] = self[_cross] + _padding[_cross];
    _contentBox[_main] = self[_main] + _padding[_main];
    _contentBox[_cross + 2] = usedSize[_cross] + _padding[_cross + 2];
    _contentBox[_main + 2] = usedSize[_main] + _padding[_main + 2];

    // Determine the flex-direction
    Direction _direction = Direction::Row;
    if (flex.direction.is(Value::Enum))
        _direction = static_cast<Direction>(static_cast<float>(flex.direction));

    // handle align-content
    Align _content = Align::Start;
    if (align.content.is(Value::Enum))
        _content = static_cast<Align>(static_cast<float>(align.content));
    float _crossStart = 0, _crossDistance = 0, _crossDir = 1;
    switch (_content) {
    case Align::Start:
        _crossStart = _contentBox[_cross];
        break;
    case Align::End:
        _crossStart = _contentBox[_cross] + _contentBox[_cross + 2];
        _crossDir = -1;
        break;
    case Align::Center:
        _crossStart = _contentBox.center()[_cross] - _usedCrossSpace / 2.f;
        break;
    case Align::Between:
        _crossStart = _contentBox[_cross];
        if (_flexLines.size() == 1) _crossDistance = 0;
        else _crossDistance = std::max(0.f, _freeCrossSpace / (_flexLines.size() - 1));
        break;
    case Align::Around:
        _crossStart = _contentBox[_cross] + (_freeCrossSpace / _flexLines.size()) / 2.;
        _crossDistance = _freeCrossSpace / _flexLines.size();
        break;
    case Align::Evenly:
        _crossStart = _contentBox[_cross] + _freeCrossSpace / (_flexLines.size() + 1);
        _crossDistance = _freeCrossSpace / (_flexLines.size() + 1);
        break;
    }

    // If direction is reverse, reverse the order of the flex-lines
    if (_direction == Direction::RowReverse || _direction == Direction::ColumnReverse)
        _crossDir *= -1, _crossStart = _contentBox.center()[_cross]
            + (_contentBox.center()[_cross] - _crossStart);

    for (auto& _line : _flexLines) {
        // Moving backwards, so remove crossSize at the start of the loop
        if (_crossDir == -1) _crossStart -= _line.crossSize;

        // Determine if we have free space in the main dimension
        float _usedSpace = 0;
        for (auto& _item : _line.items)
            _usedSpace += _item->box.addMargin(_item->box.usedSize[_main],
                _main, innerAvailableSize[_main]);
        float _freeSpace = _availableSize - _usedSpace;
        if (_freeSpace < 0) _freeSpace = 0;

        // handle justify-content
        Justify _justify = Justify::Start;
        if (justify.is(Value::Enum))
            _justify = static_cast<Justify>(static_cast<float>(justify));
        float _mainStart = 0, _mainDistance = 0, _mainDir = 1;
        switch (_justify) {
        case Justify::Start: 
            _mainStart = _contentBox[_main];
            break;
        case Justify::End: 
            _mainStart = _contentBox[_main] + _contentBox[_main + 2];
            _mainDir = -1;
            break;
        case Justify::Center: 
            _mainStart = _contentBox.center()[_main] - _usedSpace / 2.f; 
            break;
        case Justify::Between:
            _mainStart = _contentBox[_main];
            if (_line.items.size() == 1) _mainDistance = 0;
            else _mainDistance = std::max(0.f, _freeSpace / (_line.items.size() - 1));
            break;
        case Justify::Around:
            _mainStart = _contentBox[_main] + (_freeSpace / _line.items.size()) / 2.;
            _mainDistance = _freeSpace / _line.items.size();
            break;
        case Justify::Evenly:
            _mainStart = _contentBox[_main] + _freeSpace / (_line.items.size() + 1);
            _mainDistance = _freeSpace / (_line.items.size() + 1);
            break;
        }

        // If direction is reverse, reverse the order of the items in the flex-line
        if (_direction == Direction::RowReverse || _direction == Direction::ColumnReverse)
            _mainDir *= -1, _mainStart = _contentBox.center()[_main] + (_contentBox.center()[_main] - _mainStart);

        for (auto& _item : _line.items) {
            // Calculate the margin
            Vec4<float> _margin{
                _item->box.margin[0].decode(_item->box, innerAvailableSize[0]),
                _item->box.margin[1].decode(_item->box, innerAvailableSize[1]),
                _item->box.margin[2].decode(_item->box, innerAvailableSize[0]),
                _item->box.margin[3].decode(_item->box, innerAvailableSize[1]),
            };

            // Handle align-self, or align-items if align-self: auto
            Align _selfAlign = Align::Stretch;
            if (_item->box.align.self.is(Value::Enum))
                _selfAlign = static_cast<Align>(static_cast<float>(_item->box.align.self)); // Find self align
            else if (_item->box.align.self.is(Value::Auto)) // if auto, use container's item align
                _selfAlign = static_cast<Align>(static_cast<float>(align.items));
            float _crossOffset = 0;
            switch (_selfAlign) {
            case Align::End:
                _crossOffset = _line.crossSize - _item->box.usedSize[_cross];
                break;
            case Align::Center:
                _crossOffset = _line.crossSize / 2. - _item->box.usedSize[_cross] / 2. - _margin[_cross];
                break;
            }

            // Moving backwards, so remove size and margin before assigning
            if (_mainDir == -1) _mainStart -= (_item->box.usedSize[_main] + _margin[_main] + _margin[_main + 2]);

            (*_item)[_cross] = _crossStart + _margin[_cross] + _crossOffset;
            (*_item)[_main] = _mainStart + _margin[_main];
            (*_item)[_cross + 2] = _item->box.usedSize[_cross];
            (*_item)[_main + 2] = _item->box.usedSize[_main];

            // Moving forwards, so remove size and margin after assigning
            if (_mainDir == 1) _mainStart += (_item->box.usedSize[_main] + _margin[_main] + _margin[_main + 2]);
            
            // Add the distance between the items, so after assigning
            _mainStart += _mainDir * _mainDistance;
        }

        // Moving forwards, so add the cross-size at the end of the loop
        if (_crossDir == 1) _crossStart += _line.crossSize;

        // add the distance between the lines, so at the end of the loop
        _crossStart += _crossDir * _crossDistance;
    }
}

