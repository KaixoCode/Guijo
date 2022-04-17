#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Utils/Animated.hpp"

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
            Normal, // Show scrollbars when necessary
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
        struct Value { 
            enum class Type{ 
                Infinite = -3, None = -2, Auto = -1, Enum,
                Pixels, Percent, ViewWidth, ViewHeight
            };
            using enum Type;

            constexpr Value() : value(0.f), type(None) {}
            constexpr Value(px v) : value(v.value), type(Pixels) {}
            constexpr Value(pc v) : value(v.value), type(Percent) {}
            constexpr Value(vh v) : value(v.value), type(ViewHeight) {}
            constexpr Value(vw v) : value(v.value), type(ViewWidth) {}
            constexpr Value(Type t) : value(0.f), type(t) {}
            constexpr Value(float v) : value(v), type(Pixels) {}
            template<class Ty> requires std::is_enum_v<Ty>
            constexpr Value(Ty val) : value(static_cast<float>(val)), type(Enum) {}
            constexpr Value(const Value& v) : value(v.value), type(v.type) {}
            constexpr Value(Value&& v) noexcept : value(v.value), type(v.type) {}

            Value& operator=(const Value& v) { assign(v.value, v.type); return *this; }
            Value& operator=(Value&& v) noexcept { assign(v.value, v.type); return *this; }
            Value& operator=(px v) { assign(v.value, Pixels); return *this; }
            Value& operator=(pc v) { assign(v.value, Percent); return *this; }
            Value& operator=(vh v) { assign(v.value, ViewHeight); return *this; }
            Value& operator=(vw v) { assign(v.value, ViewWidth); return *this; }
            Value& operator=(Type v) { assign(0, v); return *this; }
            Value& operator=(float v) { assign(v, Pixels); return *this; }
            template<class Ty> requires std::is_enum_v<Ty>
            Value& operator=(Ty val) { assign(static_cast<float>(val), Enum); return *this; }

            constexpr bool is(Type t) const { return type == t; }
            constexpr bool definite() const { return type != Auto && type != None && type != Infinite; }

            template<class Ty> requires std::is_enum_v<Ty>
            constexpr bool operator==(Ty val) { return static_cast<float>(val) == value && type == Enum; }

            operator float() const { return value; }
            float get() const { return value; }

            Value decode(Box&, Value);

        private:
            Type type;
            Animated<float> value;
            void assign(float, Type);
        };

        struct Box {
            Point<Value> overflow{ Value::Auto, Value::Auto }; // Overflow
            Size<Value> size{ Value::Auto, Value::Auto };      // Prefered size
            Size<Value> max{ Value::None, Value::None };       // Maximum size
            Size<Value> min{ Value::None, Value::None };       // Minimum size
            Vec4<Value> margin{ 0, 0, 0, 0 };  // Margin
            Vec4<Value> padding{ 0, 0, 0, 0 }; // Padding
            Position position = Static;        // Item positioning

            struct {
                Value direction = Row;     // Flex direction
                Value basis = Value::Auto; // prefered size
                Value grow = 0;            // Proportion this item can grow relative to other items
                Value shrink = 1;          // Proportion this item can shrink relative to other items
                Wrap wrap = NoWrap;        // Wrapping mode
            } flex{};

            Value justify = Start; // Justify content (inline)
            struct {
                Value content = Stretch; // Align content (block)
                Value items = Stretch;   // Align items (Individual Items)
                Value self = Stretch;    // Align self
            } align{};

            bool use = true; // Use FlexBox sizing for children
            
            void format(Object&); // Apply FlexBox formatting to Object

            // !!Accessing any of these values below is undefined behaviour!!
            
            static inline Size<float> windowSize; // Window size, used with 'vh' and 'vw' units
            Size<Value> innerAvailableSize{}; // Available size for items (either infinite or definite)
            Size<Value> availableSize{};      // Available size for itself
            Value flexBaseSize{};             // actual value of flex-base
            Value outerFlexBaseSize{};        // actual value of flex-base + margin
            Size<Value> hypoSize{};           // flex-base clamped to min/max
            Size<Value> outerHypoSize{};      // flex-base clamped to min/max + margin
            Size<Value> targetSize{};         // target size in flex-line
            Size<Value> usedSize{};           // definitive size based on flex-line
            Box* parent = nullptr;            // Parent size, used with % unit
            bool violationType = false;       // Min-max violation when resolving flexible sizes
            bool freezeSize = false;          // Used when resolving flexible sizes in flex-line
            bool invalidated = true;          // Does this box need to be recalculated?
            
            void calcAvailableSize();
            std::size_t flowDirection();
            Value addPadding(Value, std::size_t, Value);
            Value addMargin(Value, std::size_t, Value);
            Value subPadding(Value, std::size_t, Value);
            Value subMargin(Value, std::size_t, Value);
        };
    }
}