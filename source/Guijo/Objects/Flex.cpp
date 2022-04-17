#include "Guijo/Objects/Flex.hpp"
#include "Guijo/Objects/Object.hpp"

using namespace Guijo;
using namespace Flex;

CalcValue CalcValue::decode(Box& obj, CalcValue pval) {
    switch (type) {
    // Normal/Pixels is just the value
    case Value::Pixels: return { type, value };
    // Percent uses parent size
    case Value::Percent: // If pval is percent or indefinite, we don't know...
        if (!pval.is(Value::Percent) && pval.definite()) return { Value::Pixels, pval.value * value / 100.f };
        else return { Value::Infinite };
    // View width and height are percent of window size
    case Value::ViewWidth: return { Value::Pixels, obj.windowSize[0] * value / 100.f };
    case Value::ViewHeight: return { Value::Pixels, obj.windowSize[1] * value / 100.f };
    // Remain at current value if we can't decode
    default: return *this;
    }
}

void Value::classAssign(const Value& v) {
    if (v.type != Unset) {
        type = v.type;
        if (v.type == Enum) enumValue = v.enumValue;
        else Parent::operator=(v.m_Default);
    }
    for (auto& i : v.m_Values)
        m_Values.push_back(i);

    if (v.m_Curve != Curves::linear) m_Curve = v.m_Curve;
    if (v.m_Time != 0) m_Time = v.m_Time;
}

CalcValue Margin::get(std::size_t i) {
    switch (i) {
    case 0: return { left.getType(), left.get() };
    case 1: return { top.getType(), top.get() };;
    case 2: return { right.getType(), right.get() };;
    default: return { bottom.getType(), bottom.get() };;
    }
}

CalcValue Flex::Size::get(std::size_t i) {
    switch (i) {
    case 0: return { width.getType(), width.get() };
    default: return { height.getType(), height.get() };
    }
}

CalcValue Flex::Point::get(std::size_t i) {
    switch (i) {
    case 0: return { x.getType(), x.get() };
    default: return { y.getType(), y.get() };
    }
}

void Box::operator=(const Class& v) {
    overflow.x.classAssign(v.overflow.x);
    overflow.y.classAssign(v.overflow.y);
    size.width.classAssign(v.size.width);
    size.height.classAssign(v.size.height);
    min.width.classAssign(v.min.width);
    min.height.classAssign(v.min.height);
    max.width.classAssign(v.max.width);
    max.height.classAssign(v.max.height);
    margin.left.classAssign(v.margin.left);
    margin.top.classAssign(v.margin.top);
    margin.right.classAssign(v.margin.right);
    margin.bottom.classAssign(v.margin.bottom);
    padding.left.classAssign(v.padding.left);
    padding.top.classAssign(v.padding.top);
    padding.right.classAssign(v.padding.right);
    padding.bottom.classAssign(v.padding.bottom);
    position.classAssign(v.position);
    flex.direction.classAssign(v.flex.direction);
    flex.basis.classAssign(v.flex.basis);
    flex.grow.classAssign(v.flex.grow);
    flex.shrink.classAssign(v.flex.shrink);
    flex.wrap.classAssign(v.flex.wrap);
    justify.classAssign(v.justify);
    align.content.classAssign(v.align.content);
    align.items.classAssign(v.align.items);
    align.self.classAssign(v.align.self);
}

std::size_t Box::flowDirection() {
    return flex.direction == Direction::Column
        || flex.direction == Direction::ColumnReverse;
}

void Box::calcAvailableSize() {
    auto _checkDim = [this](std::size_t dim) -> CalcValue {
        CalcValue _value{};
        CalcValue _parent = parent->innerAvailableSize[dim];
        CalcValue _size = size.get(dim).decode(*this, _parent);
        CalcValue _min = min.get(dim).decode(*this, _parent);
        CalcValue _max = max.get(dim).decode(*this, _parent);
        CalcValue _margin1 = margin.get(dim).decode(*this, _parent);
        CalcValue _margin2 = margin.get(dim + 2).decode(*this, _parent);

        // If used size is definite, content box has been defined for that dimension, so use that
        if (parent->use && usedSize[dim].definite()) _value = usedSize[dim];
        else if (parent->use && usedSize[dim].definite()) _value = usedSize[dim];
        else if (_size.definite()) _value = _size;
        else return { Value::Infinite }; // Fallback to infinite size

        // Constrain size to min/max if definite
        if (_min.definite()) _value = std::max(_value.value, _min.value);
        if (_max.definite()) _value = std::min(_value.value, _max.value);

        return _value;
    };

    if (parent == nullptr) availableSize = { windowSize[0], windowSize[1] }; // No parent at all: use window size
    else availableSize = { _checkDim(0), _checkDim(1) };

    innerAvailableSize = { 
        subPadding(availableSize[0], 0, parent ? parent->innerAvailableSize[0] : Value::Infinite),
        subPadding(availableSize[1], 0, parent ? parent->innerAvailableSize[1] : Value::Infinite)
    };
}

CalcValue Box::addPadding(CalcValue value, std::size_t dim, CalcValue pval) {
    CalcValue _value = value.decode(*this, pval);
    if (!_value.definite()) return value;
    CalcValue _padding1 = padding.get(dim).decode(*this, pval);
    CalcValue _padding2 = padding.get(dim + 2).decode(*this, pval);
    if (_padding1.definite()) value = value + _padding1;
    if (_padding2.definite()) value = value + _padding2;
    return value;
}

CalcValue Box::addMargin(CalcValue value, std::size_t dim, CalcValue pval) {
    CalcValue _value = value.decode(*this, pval);
    if (!_value.definite()) return value;
    CalcValue _margin1 = margin.get(dim).decode(*this, pval);
    CalcValue _margin2 = margin.get(dim + 2).decode(*this, pval);
    if (_margin1.definite()) value = value + _margin1;
    if (_margin2.definite()) value = value + _margin2;
    return value;
}

CalcValue Box::subPadding(CalcValue value, std::size_t dim, CalcValue pval) {
    CalcValue _value = value.decode(*this, pval);
    if (!_value.definite()) return value;
    CalcValue _padding1 = padding.get(dim).decode(*this, pval);
    CalcValue _padding2 = padding.get(dim + 2).decode(*this, pval);
    if (_padding1.definite()) value = value - _padding1;
    if (_padding2.definite()) value = value - _padding2;
    return value;
}

CalcValue Box::subMargin(CalcValue value, std::size_t dim, CalcValue pval) {
    CalcValue _value = value.decode(*this, pval);
    if (!_value.definite()) return value;
    CalcValue _margin1 = margin.get(dim).decode(*this, pval);
    CalcValue _margin2 = margin.get(dim + 2).decode(*this, pval);
    if (_margin1.definite()) value = value - _margin1;
    if (_margin2.definite()) value = value - _margin2;
    return value;
}

CalcValue clamp(Box& self, const CalcValue& pval, CalcValue v, CalcValue min, CalcValue max) {
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
    std::size_t _main = flowDirection();
    std::size_t _cross = _main == 1 ? 0 : 1;
    auto _parentSize = parent ? parent->innerAvailableSize : Vec2<CalcValue>{ windowSize[0], windowSize[1] };

    // ===================================================
    // Step 2: calculate the preferred size of all the items
    // ===================================================

    for (auto& _i : _items) {
        auto& _item = _i->box;
        _item.parent = this; // Also update parent

        // Before starting the algorithm, reset to Auto
        _item.usedSize = { Value::Auto, Value::Auto }; 
        
        // Calculate the flex-base-size of this item
        auto _flexBasis = CalcValue{ _item.flex.basis.getType(), 
            _item.flex.basis}.decode(_item, innerAvailableSize[_main]);
        auto _size = _item.size.get(_main).decode(_item, innerAvailableSize[_main]);
        if (_flexBasis.definite()) _item.flexBaseSize = _flexBasis;
        else if (_size.definite()) _item.flexBaseSize = _size;
        else _item.flexBaseSize = 0.f; // fallback to 0

        // hypoMainSize is flexBaseSize clamped to min/max values
        _item.hypoSize[_main] = clamp(_item, innerAvailableSize[_main],
            _item.flexBaseSize, _item.min.get(_main), _item.max.get(_main));

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
        bool _first = true; // Can't have 0 in a line, so first can't stop a line
        for (auto _i = _items.begin(); _i != _items.end(); ++_i) {
            auto& _item = (*_i)->box;
            float _thisSize = _item.outerHypoSize[_main];
            float _thisGrow = _item.flex.grow;
            float _thisShrink = _item.flex.shrink;
            // Check if going to overflow
            if (!_first && _usedSpace + _thisSize > _availableSize) {
                // Create line from start to item (item is end of line, so not included!)
                _flexLines.push_back({ { _start, _i },
                    _usedSpace, _flexGrow, _flexShrink });
                _usedSpace = _thisSize;
                _flexGrow = _thisGrow;
                _flexShrink = _thisShrink;
                _start = _i; // Set start of next line
            } else { // If no overflow, add to used space and grow/shrink
                _usedSpace += _thisSize;
                _flexGrow += _thisGrow;
                _flexShrink += _thisShrink;
                _first = false;
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
                        : _item.flex.grow.get();
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
                if (_item.min.get(_main).definite()) {
                    auto _min = _item.min.get(_main).decode(_item, _availableSize);
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
                if (_item.max.get(_main).definite()) {
                    auto _max = _item.max.get(_main).decode(_item, _availableSize);
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
    auto _crossSize = size.get(_cross).decode(*this, _parentSize[_cross]);
    CalcValue _innerCrossSize = _crossSize;
    if (_crossSize.definite()) {
        // Remove padding from the definite crossSize of the container
        _innerCrossSize = subPadding(_innerCrossSize, _cross, _parentSize[_cross]);
    } else if (usedSize[_cross].definite()) {
        _crossSize = usedSize[_cross];
        _innerCrossSize = subPadding(_crossSize, _cross, _parentSize[_cross]);
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
        auto _min = min.get(_cross).decode(*this, _parentSize[_cross]);
        auto _max = max.get(_cross).decode(*this, _parentSize[_cross]);
        if (_min.definite()) _flexLines[0].crossSize = std::max(_flexLines[0].crossSize, _min.value);
        if (_max.definite()) _flexLines[0].crossSize = std::min(_flexLines[0].crossSize, _max.value);
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
        for (auto& _i : _line.items) {
            auto& _item = _i->box;
            Value _selfAlign = _item.align.self; // Find self align
            if (_item.align.self.is(Value::Auto)) // if auto, use container's item align
                _selfAlign = align.items;
            if (!_selfAlign.definite()) _selfAlign = Align::Stretch; // fallback
            // If stretch, set used-cross-size to line's cross-size minus margin
            if (_selfAlign == Align::Stretch && 
                _item.size.get(_cross).is(Value::Auto)) {
                _item.usedSize[_cross] = _line.crossSize
                    - _item.margin.get(_cross)
                    - _item.margin.get(_cross + 2);
                _item.format(*_i); // Format again with new size
            } else { // No stretch, just use hypothetical-cross-size
                _item.usedSize[_cross] = _item.hypoSize[_cross];
            }
        }
    }

    // Determine container's used cross size
    auto _calcCrossSize = size.get(_cross).decode(*this, _parentSize[_cross]);
    if (_calcCrossSize.definite()) usedSize[_cross] = _calcCrossSize;
    else if (!usedSize[_cross].definite()) usedSize[_cross] = _usedCrossSpace;
    // Clamp to min/max
    usedSize[_cross] = clamp(*this, _parentSize[_cross], 
        usedSize[_cross], min.get(_cross), max.get(_cross));
    
    // ===================================================
    // Step 6: Axis-Alignment
    // ===================================================

    // Determine if we have free space in the cross dimension
    float _freeCrossSpace = usedSize[_cross] - _usedCrossSpace;
    if (_freeCrossSpace < 0) _freeCrossSpace = 0;

    // Determine content box using padding and usedSize/position
    Vec4<float> _padding{
        padding.get(0).decode(*this, _parentSize[0]),
        padding.get(1).decode(*this, _parentSize[1]),
        padding.get(2).decode(*this, _parentSize[0]),
        padding.get(3).decode(*this, _parentSize[1]),
    };
    
    // Determine the content box, so we know where to position items
    Dimensions<float> _contentBox;
    _contentBox[_cross] = self[_cross] + _padding[_cross];
    _contentBox[_main] = self[_main] + _padding[_main];
    _contentBox[_cross + 2] = usedSize[_cross] - _padding[_cross + 2] - _padding[_cross];
    _contentBox[_main + 2] = usedSize[_main] - _padding[_main + 2] - _padding[_main];

    // Determine the flex-direction
    Direction _direction = Direction::Row;
    if (flex.direction.is(Value::Enum))
        _direction = flex.direction.as<Direction>();

    // handle align-content
    Align _content = Align::Start;
    if (align.content.is(Value::Enum))
        _content = align.content.as<Align>();
    float _crossStart = _contentBox[_cross], _crossDistance = 0.f, _crossDir = 1.f;
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
        if (_flexLines.size() == 1) _crossDistance = 0.f;
        else _crossDistance = std::max(0.f, _freeCrossSpace / (_flexLines.size() - 1.f));
        break;
    case Align::Around:
        _crossStart = _contentBox[_cross] + (_freeCrossSpace / _flexLines.size()) / 2.f;
        _crossDistance = _freeCrossSpace / _flexLines.size();
        break;
    case Align::Evenly:
        _crossStart = _contentBox[_cross] + _freeCrossSpace / (_flexLines.size() + 1.f);
        _crossDistance = _freeCrossSpace / (_flexLines.size() + 1.f);
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
            _justify = justify.as<Justify>();
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

        for (auto& _i : _line.items) {
            auto& _item = _i->box;
            // Calculate the margin
            Vec4<float> _margin{
                _item.margin.get(0).decode(_item, innerAvailableSize[0]),
                _item.margin.get(1).decode(_item, innerAvailableSize[1]),
                _item.margin.get(2).decode(_item, innerAvailableSize[0]),
                _item.margin.get(3).decode(_item, innerAvailableSize[1]),
            };

            // Handle align-self, or align-items if align-self: auto
            Align _selfAlign = Align::Stretch;
            if (_item.align.self.is(Value::Enum))
                _selfAlign = _item.align.self.as<Align>(); // Find self align
            else if (_item.align.self.is(Value::Auto)) // if auto, use container's item align
                _selfAlign = align.items.as<Align>();
            float _crossOffset = 0;
            switch (_selfAlign) {
            case Align::End:
                _crossOffset = _line.crossSize - _item.usedSize[_cross];
                break;
            case Align::Center:
                _crossOffset = _line.crossSize / 2. - _item.usedSize[_cross] / 2. - _margin[_cross];
                break;
            }

            // Moving backwards, so remove size and margin before assigning
            if (_mainDir == -1) _mainStart -= (_item.usedSize[_main] + _margin[_main] + _margin[_main + 2]);

            (*_i)[_cross] = _crossStart + _margin[_cross] + _crossOffset;
            (*_i)[_main] = _mainStart + _margin[_main];
            (*_i)[_cross + 2] = _item.usedSize[_cross];
            (*_i)[_main + 2] = _item.usedSize[_main];

            // Moving forwards, so remove size and margin after assigning
            if (_mainDir == 1) _mainStart += (_item.usedSize[_main] + _margin[_main] + _margin[_main + 2]);
            
            // Add the distance between the items, so after assigning
            _mainStart += _mainDir * _mainDistance;
        }

        // Moving forwards, so add the cross-size at the end of the loop
        if (_crossDir == 1) _crossStart += _line.crossSize;

        // add the distance between the lines, so at the end of the loop
        _crossStart += _crossDir * _crossDistance;
    }
}
