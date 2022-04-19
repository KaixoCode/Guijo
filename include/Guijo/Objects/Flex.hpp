#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Utils/Animated.hpp"
#include "Guijo/Event/StateLinked.hpp"

namespace Guijo {
    class Object;
    namespace Flex {
        enum class Direction {
            Row,            // left-right
            Column,         // top-bottom
            RowReverse,     // right-left
            ColumnReverse   // bottom-top
        };
        using enum Direction;

        enum class Align {
            Center,  // centered
            Start,   // start of direction
            End,     // end of direction
            Between, // equal spacing between items
            Around,  // equal spacing around items
            Evenly,  // equal spacing between items and edge
            Stretch  // stretch items according to their grow/shrink
        };
        using enum Align;
        using Justify = Align; 
        // ^ Justify has pretty much the same values as Align

        enum class Wrap {
            DoWrap,     // Wrap elements
            NoWrap,     // Don't wrap elements (overflow if necessary)
            WrapReverse // Reverse the block direction
        };
        using enum Wrap;

        enum class Position {
            Static,   // Default position
            Relative, // Relative to default position
            Absolute, // Relative to first positioned (not static) ancestor
            Fixed     // Relative to window
        };
        using enum Position;

        enum class Overflow {
            Scroll, // Always show scrollbars
            Hidden, // Clip overflowing content, no scrollbar
            Visible // Don't clip overflowing content
        };
        using enum Overflow;

        struct px { float value; }; // Pixels
        struct pc { float value; }; // Percentage
        struct vh { float value; }; // View height
        struct vw { float value; }; // View width
        struct Box;
        class Value : public StateLinked<Animated<float>> {
            using Parent = StateLinked<Animated<float>>;
        public:
            enum class Type : std::int8_t { 
                Unset = -4, Infinite = -3, None = -2, Auto = -1, 
                Enum, Pixels, Percent, ViewWidth, ViewHeight, 
            };
            using enum Type;

            constexpr Value() : Parent{ 0.f }, type(None) {}
            constexpr Value(px v) : Parent{ v.value }, type(Pixels) {}
            constexpr Value(pc v) : Parent{ v.value }, type(Percent) {}
            constexpr Value(vh v) : Parent{ v.value }, type(ViewHeight) {}
            constexpr Value(vw v) : Parent{ v.value }, type(ViewWidth) {}
            constexpr Value(Type t) : Parent{ 0.f }, type(t) {}
            constexpr Value(float v) : Parent{ v }, type(Pixels) {}
            constexpr Value(const Value& v) = default;
            constexpr Value(Value&& v) = default;

            template<class Ty> requires std::is_enum_v<Ty>
            constexpr Value(Ty val) : enumValue(static_cast<float>(val)), type(Enum) {}

            constexpr Value& operator=(const Value& v) = default;
            constexpr Value& operator=(Value&& v) noexcept = default;
            constexpr Value& operator=(px v) { return assign(v.value, Pixels); }
            constexpr Value& operator=(pc v) { return assign(v.value, Percent); }
            constexpr Value& operator=(vh v) { return assign(v.value, ViewHeight); }
            constexpr Value& operator=(vw v) { return assign(v.value, ViewWidth); }
            constexpr Value& operator=(Type v) { return assign(0, v); }
            constexpr Value& operator=(float v) { return assign(v, Pixels); }
            
            template<class Ty> requires std::is_enum_v<Ty>
            constexpr Value& operator=(Ty val) {
                enumValue = static_cast<float>(val);
                type = Enum;
                return *this;
            }

            constexpr Type getType() const { return type; }

            constexpr bool is(Type t) const { return type == t; }
            constexpr bool definite() const { return static_cast<std::int8_t>(type) >= 0; }

            template<class Ty> requires std::is_enum_v<Ty>
            constexpr bool operator==(Ty val) const {
                return static_cast<float>(val) == enumValue && type == Enum;
            }

            template<class Ty> requires std::is_enum_v<Ty>
            constexpr Ty as() const { return static_cast<Ty>(enumValue); }

        private:
            Type type;
            float enumValue{};

            constexpr Value& assign(float newval, Type newtype) {
                if (newtype != type) value(newval), type = newtype;
                else value(newval);
                return *this;
            }

            void classAssign(const Value& v);

            friend class EventReceiver;
            friend class Box;
        };

        class CalcValue{
        public:
            using enum Value::Type;

            constexpr CalcValue() : value(0.f), type(None) {}
            constexpr CalcValue(Value::Type t, float v) : value(v), type(t) {}
            constexpr CalcValue(Value::Type t) : value(0.f), type(t) {}
            constexpr CalcValue(float v) : value(v), type(Pixels) {}
        
            constexpr void operator=(float v) { value = v, type = Pixels; }
            constexpr operator float() const { return value; }

            constexpr bool is(Value::Type t) const { return type == t; }
            constexpr bool definite() const { return static_cast<std::int8_t>(type) >= 0; }

            Value::Type type;
            float value;

            CalcValue decode(Box&, CalcValue);
        };

        struct Margin {
            Value left;
            Value top;
            Value right;
            Value bottom;

            constexpr operator Vec4<float>() const {
                return { left, top, right, bottom };
            }

            constexpr Margin& operator=(const Vec4<float>& v) {
                left = v[0];
                top = v[1];
                right = v[2];
                bottom = v[3];
                return *this;
            }

            constexpr Margin& operator=(float v) { return operator=(Vec4<float>{ v, v, v, v }); };

            struct Assigner {
                Margin& margin;
                StateLink state;
                constexpr void operator=(float v) {
                    margin.left[state] = v;
                    margin.right[state] = v;
                    margin.top[state] = v;
                    margin.bottom[state] = v;
                }
            };

            constexpr Assigner operator[](StateId id) { return Assigner{ *this, { id } }; }
            constexpr Assigner operator[](StateLink link) { return Assigner{ *this, link }; }

            constexpr void curve(Curve curve) { left.curve(curve), top.curve(curve), right.curve(curve), bottom.curve(curve); };
            constexpr void transition(double millis) { left.transition(millis), top.transition(millis), right.transition(millis), bottom.transition(millis); }
            constexpr void jump(float val) { left.jump(val), top.jump(val), right.jump(val), bottom.jump(val); }

        private:
            CalcValue get(std::size_t i);
            friend class Box;
        };
        using Padding = Margin;

        struct Size {
            Value width;
            Value height;

            constexpr operator Vec2<float>() const {
                return { width, height };
            }

            constexpr Size& operator=(const Vec2<float>& v) {
                width = v[0];
                height = v[1];
                return *this;
            }

            constexpr Size& operator=(float v) { return operator=(Vec2<float>{ v, v }); };

            struct Assigner {
                Size& margin;
                StateLink state;
                constexpr void operator=(float v) {
                    margin.width[state] = v;
                    margin.height[state] = v;
                }
            };

            constexpr Assigner operator[](StateId id) { return Assigner{ *this, { id } }; }
            constexpr Assigner operator[](StateLink link) { return Assigner{ *this, link }; }

            constexpr void curve(Curve curve) { width.curve(curve), height.curve(curve); };
            constexpr void transition(double millis) { width.transition(millis), height.transition(millis); }
            constexpr void jump(float val) { width.jump(val), height.jump(val); }

        private:
            CalcValue get(std::size_t i);
            friend class Box;
        };

        struct Point {
            Value x;
            Value y;

            constexpr operator Vec2<float>() const {
                return { x, y };
            }

            constexpr Flex::Point& operator=(const Vec2<float>& v) {
                x = v[0];
                y = v[1];
                return *this;
            }

            constexpr Point& operator=(float v) { return operator=(Vec2<float>{ v, v }); };

        private:
            CalcValue get(std::size_t i);
            friend class Box;
        };

        struct Class {
            Point overflow{ Value::Unset, Value::Unset }; // Overflow
            Size size{ Value::Unset, Value::Unset };      // Prefered size
            Size max{ Value::Unset, Value::Unset };       // Maximum size
            Size min{ Value::Unset, Value::Unset };       // Minimum size
            Margin margin{ Value::Unset, Value::Unset, Value::Unset, Value::Unset };   // Margin
            Padding padding{ Value::Unset, Value::Unset, Value::Unset, Value::Unset }; // Padding
            Value position = Value::Unset;  // Item positioning

            struct {
                Value direction = Value::Unset;  // Flex direction
                Value basis = Value::Unset;      // prefered size
                Value grow = Value::Unset;       // Proportion this item can grow relative to other items
                Value shrink = Value::Unset;     // Proportion this item can shrink relative to other items
                Value wrap = Value::Unset;       // Wrapping mode
            } flex{};

            Value justify = Value::Unset; // Justify content (inline)
            struct {
                Value content = Value::Unset; // Align content (block)
                Value items = Value::Unset;   // Align items (Individual Items)
                Value self = Value::Unset;    // Align self
            } align{};
        };

        struct Box {
            Point overflow{ Value::Auto, Value::Auto }; // Overflow
            Size size{ Value::Auto, Value::Auto };      // Prefered size
            Size max{ Value::None, Value::None };       // Maximum size
            Size min{ Value::None, Value::None };       // Minimum size
            Margin margin{ 0, 0, 0, 0 };                // Margin
            Padding padding{ 0, 0, 0, 0 };              // Padding
            Value position = Static;                    // Item positioning

            struct {
                Value direction = Row;     // Flex direction
                Value basis = Value::Auto; // prefered size
                Value grow = 0;            // Proportion this item can grow relative to other items
                Value shrink = 1;          // Proportion this item can shrink relative to other items
                Value wrap = NoWrap;       // Wrapping mode
            } flex{};

            Value justify = Start; // Justify content (inline)
            struct {
                Value content = Stretch;  // Align content (block)
                Value items = Stretch;    // Align items (Individual Items)
                Value self = Value::Auto; // Align self
            } align{};

            bool use = true; // Use FlexBox sizing for children
            
            void format(Object&, bool sizing = false); // Apply FlexBox formatting to Object

            void operator=(const Class&);

            static inline Vec2<float> windowSize; // Window size, used with 'vh' and 'vw' units
        private:
            Vec2<CalcValue> innerAvailableSize{}; // Available size for items (either infinite or definite)
            Vec2<CalcValue> availableSize{};      // Available size for itself
            CalcValue flexBaseSize{};             // actual value of flex-base
            CalcValue outerFlexBaseSize{};        // actual value of flex-base + margin
            Vec2<CalcValue> hypoSize{};           // flex-base clamped to min/max
            Vec2<CalcValue> outerHypoSize{};      // flex-base clamped to min/max + margin
            Vec2<CalcValue> targetSize{};         // target size in flex-line
            Vec2<CalcValue> usedSize{};           // definitive size based on flex-line
            Box* parent = nullptr;            // Parent size, used with % unit
            bool violationType = false;       // Min-max violation when resolving flexible sizes
            bool freezeSize = false;          // Used when resolving flexible sizes in flex-line
            bool invalidated = true;          // Does this box need to be recalculated?
            
            void calcAvailableSize();
            std::size_t flowDirection();
            CalcValue addPadding(CalcValue, std::size_t, CalcValue);
            CalcValue addMargin(CalcValue, std::size_t, CalcValue);
            CalcValue subPadding(CalcValue, std::size_t, CalcValue);
            CalcValue subMargin(CalcValue, std::size_t, CalcValue);
            friend class CalcValue;
            friend class Window;
        };
    }
}