#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"

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
                Infinite = -3, None = -2, Auto = -1, Normal, 
                Pixels, Percent, ViewWidth, ViewHeight, Content
            };
            using enum Type;

            constexpr Value() : value(0), type(None) {}
            constexpr Value(px v) : value(v.value), type(Pixels) {}
            constexpr Value(pc v) : value(v.value), type(Percent) {}
            constexpr Value(vh v) : value(v.value), type(ViewHeight) {}
            constexpr Value(vw v) : value(v.value), type(ViewWidth) {}
            constexpr Value(Type t) : value(0), type(t) {}
            constexpr Value(float v) : value(v), type(Normal) {}

            constexpr Value& operator=(float v) { value = v, type = Normal; return *this; }

            constexpr bool is(Type t) const { return type == t; }
            constexpr bool definite() const { return type != Auto && type != None && type != Infinite; }

            Type type;
            float value;

            Value decode(Box&, Value);
        };

        struct Box {
            Point<Overflow> overflow{ Value::Auto, Value::Auto }; // Overflow
            Size<Value> size{ Value::Auto, Value::Auto };          // Prefered size
            Size<Value> max{ Value::None, Value::None };           // Maximum size
            Size<Value> min{ Value::None, Value::None };           // Minimum size
            Vec4<Value> margin{ 0, 0, 0, 0 };        // Margin
            Vec4<Value> padding{ 0, 0, 0, 0 };       // Padding
            Position position = Static;             // Item positioning

            struct {
                Direction direction = Row; // Flex direction
                Value basis = Value::Auto; // prefered size
                float grow = 0;            // Proportion this item can grow relative to other items
                float shrink = 1;          // Proportion this item can shrink relative to other items
                Wrap wrap = NoWrap;        // Wrapping mode
            } flex{};

            Justify justify = Start; // Justify content (inline)
            struct {
                Align content = Stretch; // Align content (block)
                Align items = Stretch;   // Align items (Individual Items)
            } align{};

            bool use = true; // Use FlexBox sizing for children
            
            void format(Object&); // Apply FlexBox formatting to Object

            // !!Accessing any of these values below is undefined behaviour!!
            
            static inline Size<float> windowSize; // Window size, used with 'vh' and 'vw' units
            Size<Value> availableSize{};     // Available size for items
            Value flexBaseSize{};            // actual value of flex-base
            Value outerFlexBaseSize{};       // actual value of flex-base
            Value hypoMainSize{};            // flex-base clamped to min/max
            Value outerHypoMainSize{};       // flex-base clamped to min/max + margin
            Value targetMainSize{};          // target size in flex-line
            float usedMainSize{};            // definitive size based on flex-line
            Value hypoCrossSize{};           // 
            float usedCrossSize{};           // 
            Box* parent = nullptr;           // Parent size, used with % unit
            bool violationType = false;      // Min-max violation when resolving flexible sizes
            bool freezeSize = false;         // Used when resolving flexible sizes in flex-line
            bool invalidated = true;         // Does this box need to be recalculated?
            

            void calcAvailableSize();
            void calcPreferredSize();
            int flowDirection();
        };
    }
}