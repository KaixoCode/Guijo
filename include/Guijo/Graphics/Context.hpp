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

	template<class ...Tys> 
		requires ((std::is_trivially_destructible_v<std::decay_t<Tys>> && ...))
	std::unique_ptr<uint8_t[]> make(Tys&&...args) {
		std::unique_ptr<uint8_t[]> _ptr = std::make_unique<uint8_t[]>((sizeof(Tys) + ...));
		uint8_t* _data = _ptr.get();
		std::size_t _offset = 0;
		((std::memcpy(&_data[_offset], &args, sizeof(Tys)), _offset += sizeof(Tys)), ...);
		return std::move(_ptr);
	}

	struct CommandData {
		template<class Ty> CommandData(const Ty& val)
			: type(typeid(Ty)), data(make(val)) {}

		const std::type_info& type;
		std::unique_ptr<uint8_t[]> data;

		template<class Ty> Ty& get() { return *reinterpret_cast<Ty*>(data.get()); }
	};

	enum Commands : std::size_t {
		Fill, Rect, Line, Ellipse, Triangle, 
		Text, FontSize, Font, TextAlign, LineHeight,
		Translate, PushMatrix, PopMatrix, Viewport,
		Clip, PushClip, PopClip, ClearClip,
		Amount
	};

	template<std::size_t Is> struct Command;

	template<> struct Command<Fill> { Color color; };
	template<> struct Command<Rect> { Vec4<double> rect; double rotation; };
	template<> struct Command<Line> { Vec4<double> line; double thickness = 1; };
	template<> struct Command<Ellipse> { Vec4<double> ellipse; Vec2<double> angles{ 0, 0 }; };
	template<> struct Command<Triangle> { Vec4<double> rect; double rotation; };
	template<> struct Command<Text> { std::string_view text; Vec2<double> pos; };
	template<> struct Command<FontSize> { double size; };
	template<> struct Command<Font> { std::string_view font; };
	template<> struct Command<TextAlign> { int align; };
	template<> struct Command<LineHeight> { double height; };
	template<> struct Command<Translate> { Vec2<double> translate; };
	template<> struct Command<PushMatrix> { };
	template<> struct Command<PopMatrix> { };
	template<> struct Command<Viewport> { Vec4<double> rect; };
	template<> struct Command<Clip> { Vec4<double> clip; };
	template<> struct Command<PushClip> { };
	template<> struct Command<PopClip> { };
	template<> struct Command<ClearClip> { };

	class DrawContext {
		friend class GraphicsBase;
	public:
		void fill(Command<Fill> v) { m_Commands.emplace(v); }
		void rect(Command<Rect> v) { m_Commands.emplace(v); }
		void line(Command<Line> v) { m_Commands.emplace(v); }
		void ellipse(Command<Ellipse> v) { m_Commands.emplace(v); }
		void triangle(Command<Triangle> v) { m_Commands.emplace(v); }
		void text(Command<Text> v) { m_Commands.emplace(v); }
		void fontSize(Command<FontSize> v) { m_Commands.emplace(v); }
		void font(Command<Font> v) { m_Commands.emplace(v); }
		void textAlign(Command<TextAlign> v) { m_Commands.emplace(v); }
		void lineHeight(Command<LineHeight> v) { m_Commands.emplace(v); }
		void translate(Command<Translate> v) { m_Commands.emplace(v); }
		void pushMatrix(Command<PushMatrix> v) { m_Commands.emplace(v); }
		void popMatrix(Command<PopMatrix> v) { m_Commands.emplace(v); }
		void viewport(Command<Viewport> v) { m_Commands.emplace(v); }
		void clip(Command<Clip> v) { m_Commands.emplace(v); }
		void pushClip(Command<PushClip> v) { m_Commands.emplace(v); }
		void popClip(Command<PopClip> v) { m_Commands.emplace(v); }
		void clearClip(Command<ClearClip> v) { m_Commands.emplace(v); }

	private:
		std::queue<CommandData> m_Commands;
	};
}