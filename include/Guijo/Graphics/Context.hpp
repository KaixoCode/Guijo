#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Utils/Color.hpp"
#include "Guijo/Utils/Angle.hpp"

namespace Guijo {
    enum class Align {
        Left    = 0x00, // aligned to left on x-axis
        Right   = 0x01, // aligned to right on x-axis
        CenterX = 0x02, // aligned to center on x-axis
        Top     = 0x00, // aligned to top on y-axis
        Bottom  = 0x10, // aligned to bottom on y-axis
        CenterY = 0x20, // aligned to center on y-axis
        Center  = 0x22, // aligned to center on both axis

        TextTop    = Top,
        TextBottom = Bottom,
        Baseline   = 0x40,
        Middle     = CenterY,
    };
    using Alignment = std::uint8_t;
    enum class StrokeCap {
        Round = 0, // (====)  Rounded off
        Square,    //  ====   Cut off at point
        Project,   // ======  Cut off at point + strokeWeight
    };

    struct MemoryPool {
        std::vector<uint8_t> data;
        std::size_t counter = 0ull;

        MemoryPool() {
            data.resize(100);
        }

        uint8_t* allocate(std::size_t& index, std::size_t bytes) {
            if (counter + bytes >= data.size()) 
                data.resize(counter + bytes + 1);
            std::uint8_t* _ptr = &data[counter];
            index = counter;
            counter += bytes;
            return _ptr;
        }

        void reset() { counter = 0; }
    };

    // Converts trivially destructible types into a unique ptr of raw bytes
    template<class ...Tys> 
        requires ((std::is_trivially_destructible_v<std::decay_t<Tys>> && ...))
    std::size_t make(MemoryPool& pool, Tys&&...args) {
        std::size_t _index;
        uint8_t* _data = pool.allocate(_index, (sizeof(Tys) + ...));
        std::size_t _offset = 0;
        ((std::memcpy(&_data[_offset], &args, sizeof(Tys)), _offset += sizeof(Tys)), ...);
        return _index;
    }

    enum class Commands : std::size_t {
        Fill = 0, Stroke, StrokeWeight, Rect, Line, Circle, Triangle, 
        Text, FontSize, SetFont, TextAlign,
        Translate, PushMatrix, PopMatrix, Viewport,
        Clip, PushClip, PopClip, ClearClip,
        Amount
    };
    using enum Commands;

    template<Commands Is> struct Command;
    template<> struct Command<Fill> { Color color; };
    template<> struct Command<Stroke> { Color color; };
    template<> struct Command<StrokeWeight> { float weight; };
    template<> struct Command<Rect> { Dimensions<float> dimensions; Vec4<float> radius = { 0, 0, 0, 0 }; Angle<float> rotation = 0; };
    template<> struct Command<Line> { Point<float> start; Point<float> end; StrokeCap cap = StrokeCap::Round; };
    template<> struct Command<Circle> { Point<float> center; float radius; Vec2<Angle<float>> angles{ 0, 0 }; };
    template<> struct Command<Triangle> { Point<float> a; Point<float> b; Point<float> c; };
    template<> struct Command<Text> { std::string_view text; Point<float> pos; };
    template<> struct Command<FontSize> { float size; };
    template<> struct Command<SetFont> { std::string_view font; };
    template<> struct Command<TextAlign> { Alignment align; };
    template<> struct Command<Translate> { Point<float> translate; };
    template<> struct Command<PushMatrix> { };
    template<> struct Command<PopMatrix> { };
    template<> struct Command<Viewport> { Dimensions<float> viewport; };
    template<> struct Command<Clip> { Dimensions<float> clip; };
    template<> struct Command<PushClip> { };
    template<> struct Command<PopClip> { };
    template<> struct Command<ClearClip> { };

    struct CommandData {
        template<Commands Ty> CommandData(MemoryPool& pool, const Command<Ty>& val)
            : pool(pool), type(Ty), index(make(pool, val)) {}

        MemoryPool& pool;
        Commands type;
        std::size_t index;

        template<Commands Ty> Command<Ty>& get() {
            return *reinterpret_cast<Command<Ty>*>(&pool.data[index]);
        }
    };

    class DrawContext {
        friend class GraphicsBase;
    public:
        void fill(const Command<Fill>& v) { m_Commands.emplace_back(memPool, v); }
        void stroke(const Command<Stroke>& v) { m_Commands.emplace_back(memPool, v); }
        void strokeWeight(const Command<StrokeWeight>& v) { m_Commands.emplace_back(memPool, v); }
        void noStroke() { m_Commands.emplace_back(memPool, Command<StrokeWeight>{ 0 }); }
        void rect(const Command<Rect>& v) { m_Commands.emplace_back(memPool, v); }
        void line(const Command<Line>& v) { m_Commands.emplace_back(memPool, v); }
        void circle(const Command<Circle>& v) { m_Commands.emplace_back(memPool, v); }
        void triangle(const Command<Triangle>& v) { m_Commands.emplace_back(memPool, v); }
        void text(const Command<Text>& v) { m_Commands.emplace_back(memPool, v); }
        void fontSize(const Command<FontSize>& v) { m_Commands.emplace_back(memPool, v); }
        void font(const Command<SetFont>& v) { m_Commands.emplace_back(memPool, v); }
        void textAlign(const Command<TextAlign>& v) { m_Commands.emplace_back(memPool, v); }
        void translate(const Command<Translate>& v) { m_Commands.emplace_back(memPool, v); }
        void pushMatrix() { m_Commands.emplace_back(memPool, Command<PushMatrix>{}); }
        void popMatrix() { m_Commands.emplace_back(memPool, Command<PopMatrix>{}); }
        void viewport(const Command<Viewport>& v) { m_Commands.emplace_back(memPool, v); }
        void clip(const Command<Clip>& v) { m_Commands.emplace_back(memPool, v); }
        void pushClip() { m_Commands.emplace_back(memPool, Command<PushClip>{}); }
        void popClip() { m_Commands.emplace_back(memPool, Command<PopClip>{}); }
        void clearClip() { m_Commands.emplace_back(memPool, Command<ClearClip>{}); }

        void fill(const Color& v) {
            m_Commands.emplace_back(memPool, Command<Fill>{ v });
        }
        
        void stroke(const Color& v) {
            m_Commands.emplace_back(memPool, Command<Stroke>{ v });
        }
        
        void strokeWeight(float v) {
            m_Commands.emplace_back(memPool, Command<StrokeWeight>{ v });
        }

        void rect(const Dimensions<float>& rect, const Dimensions<float> radius = 0, Angle<float> rotation = 0) {
            m_Commands.emplace_back(memPool, Command<Rect>{ rect, radius, rotation });
        }

        void line(const Point<float>& start, const Point<float>& end, StrokeCap cap = StrokeCap::Round) {
            m_Commands.emplace_back(memPool, Command<Line>{ start, end, cap });
        }

        void circle(const Point<float>& center, float radius, const Vec2<Angle<float>>& angles = { 0, 0 }) {
            m_Commands.emplace_back(memPool, Command<Circle>{ center, radius, angles });
        }

        void triangle(const Point<float>& a, const Point<float> b, const Point<float> c) {
            m_Commands.emplace_back(memPool, Command<Triangle>{ a, b, c });
        }

        void text(std::string_view text, const Point<float>& pos) { 
            m_Commands.emplace_back(memPool, Command<Text>{ text, pos });
        }

        void fontSize(float size) { 
            m_Commands.emplace_back(memPool, Command<FontSize>{ size });
        }

        void font(std::string_view font) {
            m_Commands.emplace_back(memPool, Command<SetFont>{ font });
        }

        void textAlign(Alignment align) { 
            m_Commands.emplace_back(memPool, Command<TextAlign>{ align });
        }

        void textAlign(Align align) { 
            m_Commands.emplace_back(memPool, Command<TextAlign>{ static_cast<Alignment>(align) });
        }

        void translate(const Point<float>& translate) {
            m_Commands.emplace_back(memPool, Command<Translate>{ translate });
        }

        void viewport(const Dimensions<float>& viewport) {
            m_Commands.emplace_back(memPool, Command<Viewport>{ viewport });
        }

        void clip(Dimensions<float> clip) { 
            m_Commands.emplace_back(memPool, Command<Clip>{ clip });
        }

    private:
        MemoryPool memPool;
        std::vector<CommandData> m_Commands;
    };
}