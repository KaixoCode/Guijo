#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"

namespace Guijo {
    class Object;
    namespace Flex {
        enum Values { Auto = -1, None = -2 };
        using enum Values;

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
        struct Unit { 
            enum class Type{ Pixels, Percent, ViewWidth, ViewHeight, Auto, None };
            using enum Type;

            constexpr Unit(px v) : value(v.value), type(Pixels) {}
            constexpr Unit(pc v) : value(v.value), type(Percent) {}
            constexpr Unit(vh v) : value(v.value), type(ViewHeight) {}
            constexpr Unit(vw v) : value(v.value), type(ViewWidth) {}
            constexpr Unit(float v) : value(v), 
                type(v == Values::Auto ? Auto : v == Values::None ? None : Pixels) {}

            constexpr bool is(Type t) const { return type == t; }
            constexpr bool definite() const { return type != Auto && type != None; }

            Type type;
            float value;

            float decode(Box&, bool dir);
        };

        struct Box {
            Point<Overflow> overflow{ Auto, Auto }; // Overflow
            Size<Unit> size{ Auto, Auto };          // Prefered size
            Size<Unit> max{ None, None };           // Maximum size
            Size<Unit> min{ None, None };           // Minimum size
            Vec4<Unit> margin{ 0, 0, 0, 0 };        // Margin
            Vec4<Unit> padding{ 0, 0, 0, 0 };       // Padding
            Position position = Static;             // Item positioning

            struct {
                Direction direction = Row; // Flex direction
                Unit basis = Auto;         // prefered size
                float grow = 0;            // Proportion this item can grow relative to other items
                float shrink = 1;          // Proportion this item can shrink relative to other items
                Wrap wrap = NoWrap;        // Wrapping mode
            } flex{};

            Justify justify = Start; // Justify content (inline)
            struct {
                Align content = Stretch; // Align content (block)
                Align items = Stretch;   // Align items (Individual Items)
            } align{};

            bool use = false; // Use FlexBox sizing for children
            
            void format(Object&); // Apply FlexBox formatting to Object

        protected:
            static inline Size<float> windowSize; // Window size, used with 'vh' and 'vw' units
            Size<float> finalSize;          // final calculated size
            Size<float> innerSize;     // final calculated size, taking into account padding
            float hypoMainSize;        // preferred size of object
            float outerHypoMainSize;   // preferred size of object
            Size<float> parentSize;    // Parent size, used with % unit
            bool invalidated = true;   // Does this box need to be recalculated?
            
            void refreshPreferredSize(int axis);
            friend class Unit;
        };
    }
}