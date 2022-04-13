#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"
#include "Guijo/Utils/Color.hpp"

namespace Guijo {
	namespace Align {
		enum {
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
	}
	using Alignment = std::uint8_t;
	enum class StrokeCap {
		Round = 0, // (====)  Rounded off
		Square,    //  ====   Cut off at point
		Project,   // ======  Cut off at point + strokeWeight
	};

	// Converts trivially destructible types into a unique ptr of raw bytes
	template<class ...Tys> 
		requires ((std::is_trivially_destructible_v<std::decay_t<Tys>> && ...))
	std::unique_ptr<uint8_t[]> make(Tys&&...args) {
		std::unique_ptr<uint8_t[]> _ptr = std::make_unique<uint8_t[]>((sizeof(Tys) + ...));
		uint8_t* _data = _ptr.get();
		std::size_t _offset = 0;
		((std::memcpy(&_data[_offset], &args, sizeof(Tys)), _offset += sizeof(Tys)), ...);
		return std::move(_ptr);
	}

	enum Commands : std::size_t {
		Fill = 0, Stroke, StrokeWeight, Rect, Line, Circle, Triangle, 
		Text, FontSize, SetFont, TextAlign,
		Translate, PushMatrix, PopMatrix, Viewport,
		Clip, PushClip, PopClip, ClearClip,
		Amount
	};

	template<std::size_t Is> struct Command;
	template<> struct Command<Fill> { Color color; };
	template<> struct Command<Stroke> { Color color; };
	template<> struct Command<StrokeWeight> { float weight; };
	template<> struct Command<Rect> { Dimensions<float> dimensions; Dimensions<float> radius = 0; float rotation = 0;  };
	template<> struct Command<Line> { Point<float> start; Point<float> end; StrokeCap cap = StrokeCap::Round; };
	template<> struct Command<Circle> { Point<float> center; float radius; Point<float> angles{ 0, 0 };  };
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
		template<std::size_t Ty> CommandData(const Command<Ty>& val)
			: type(Ty), data(make(val)) {}

		std::size_t type;
		std::unique_ptr<uint8_t[]> data;

		template<std::size_t Ty> Command<Ty>& get() { 
			return *reinterpret_cast<Command<Ty>*>(data.get()); 
		}
	};

	class DrawContext {
		friend class GraphicsBase;
	public:
		void fill(const Command<Fill>& v) { m_Commands.emplace(v); }
		void stroke(const Command<Stroke>& v) { m_Commands.emplace(v); }
		void strokeWeight(const Command<StrokeWeight>& v) { m_Commands.emplace(v); }
		void noStroke() { m_Commands.emplace(Command<StrokeWeight>{ 0 }); }
		void rect(const Command<Rect>& v) { m_Commands.emplace(v); }
		void line(const Command<Line>& v) { m_Commands.emplace(v); }
		void circle(const Command<Circle>& v) { m_Commands.emplace(v); }
		void triangle(const Command<Triangle>& v) { m_Commands.emplace(v); }
		void text(const Command<Text>& v) { m_Commands.emplace(v); }
		void fontSize(const Command<FontSize>& v) { m_Commands.emplace(v); }
		void font(const Command<SetFont>& v) { m_Commands.emplace(v); }
		void textAlign(const Command<TextAlign>& v) { m_Commands.emplace(v); }
		void translate(const Command<Translate>& v) { m_Commands.emplace(v); }
		void pushMatrix() { m_Commands.emplace(Command<PushMatrix>{}); }
		void popMatrix() { m_Commands.emplace(Command<PopMatrix>{}); }
		void viewport(const Command<Viewport>& v) { m_Commands.emplace(v); }
		void clip(const Command<Clip>& v) { m_Commands.emplace(v); }
		void pushClip() { m_Commands.emplace(Command<PushClip>{}); }
		void popClip() { m_Commands.emplace(Command<PopClip>{}); }
		void clearClip() { m_Commands.emplace(Command<ClearClip>{}); }

		void fill(const Color& v) {
			m_Commands.emplace(Command<Fill>{ v });
		}
		
		void stroke(const Color& v) {
			m_Commands.emplace(Command<Stroke>{ v });
		}
		
		void strokeWeight(float v) {
			m_Commands.emplace(Command<StrokeWeight>{ v });
		}

		void rect(const Dimensions<float>& rect, const Dimensions<float> radius = 0, float rotation = 0) {
			m_Commands.emplace(Command<Rect>{ rect, radius, rotation });
		}

		void line(const Point<float>& start, const Point<float>& end, StrokeCap cap = StrokeCap::Round) {
			m_Commands.emplace(Command<Line>{ start, end, cap }); 
		}

		void circle(const Point<float>& center, float radius, const Point<float>& angles = { 0, 0 }) {
			m_Commands.emplace(Command<Circle>{ center, radius, angles });
		}

		void triangle(const Point<float>& a, const Point<float> b, const Point<float> c) {
			m_Commands.emplace(Command<Triangle>{ a, b, c });
		}

		void text(std::string_view text, const Point<float>& pos) { 
			m_Commands.emplace(Command<Text>{ text, pos });
		}

		void fontSize(float size) { 
			m_Commands.emplace(Command<FontSize>{ size });
		}

		void font(std::string_view font) {
			m_Commands.emplace(Command<SetFont>{ font });
		}

		void textAlign(Alignment align) { 
			m_Commands.emplace(Command<TextAlign>{ align });
		}

		void translate(const Point<float>& translate) {
			m_Commands.emplace(Command<Translate>{ translate });
		}

		void viewport(const Dimensions<float>& viewport) {
			m_Commands.emplace(Command<Viewport>{ viewport });
		}

		void clip(Dimensions<float> clip) { 
			m_Commands.emplace(Command<Clip>{ clip });
		}

	private:
		std::queue<CommandData> m_Commands;
	};
}