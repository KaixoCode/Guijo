#pragma once
#include "Guijo/Graphics/Context.hpp"
#include "Guijo/Graphics/Font.hpp"
#include "Guijo/Graphics/Shader.hpp"

namespace Guijo {
	class GraphicsBase {
		static inline int m_GraphicsIdCounter = 0;
	protected:
		static inline std::map<std::string, Guijo::FontType, std::less<>> m_Fonts;
	public:
		GraphicsBase() = default;
		GraphicsBase(GraphicsBase&&) = delete;
		GraphicsBase(const GraphicsBase&) = delete;

		void render(DrawContext& context);

		static inline std::string DefaultFont = "segoeui";
		static void loadFont(const std::string& path, const std::string& name);
		static bool loadFont(const std::string& name);
		static float charWidth(const char c, const std::string_view& font, float size);
		static float stringWidth(const std::string_view& c, const std::string_view& font, float size);
		
		virtual void scale(Vec2<float>);
		virtual void dimensions(const Dimensions<float>&);
		virtual void prepare() = 0; // Prepare for drawing (i.e. context switching)
		virtual void swapBuffers() = 0;

	protected:
		std::stack<Dimensions<float>> m_ClipStack;
		Dimensions<float> m_Clip{};

		glm::mat4 m_Projection{ 0.f };
		glm::mat4 m_ViewProj{ 0.f };
		std::stack<glm::mat4> m_MatrixStack;
		glm::mat4 m_Matrix{ 1.0f };
		Vec2<float> m_Size{};
		int m_GraphicsId = m_GraphicsIdCounter++;
		float m_Scaling = 1;

		FontType* m_CurrentFont = nullptr;

		template<std::size_t ...Is>
		void runCommand(CommandData& command, std::index_sequence<Is...>) {
			((command.type == typeid(Command<Is>)
				? (runCommand(command.get<Command<Is>>())
					, true) : false) || ...);
		}

		virtual void runCommand(Command<Fill>&) = 0;
		virtual void runCommand(Command<Rect>&) = 0;
		virtual void runCommand(Command<Line>&) = 0;
		virtual void runCommand(Command<Ellipse>&) = 0;
		virtual void runCommand(Command<Triangle>&) = 0;
		virtual void runCommand(Command<Text>&) = 0;
		virtual void runCommand(Command<FontSize>&) = 0;
		virtual void runCommand(Command<Font>&) = 0;
		virtual void runCommand(Command<TextAlign>&) = 0;
		virtual void runCommand(Command<LineHeight>&) = 0;
		virtual void runCommand(Command<Translate>&) = 0;
		virtual void runCommand(Command<PushMatrix>&) = 0;
		virtual void runCommand(Command<PopMatrix>&) = 0;
		virtual void runCommand(Command<Viewport>&) = 0;
		virtual void runCommand(Command<Clip>&) = 0;
		virtual void runCommand(Command<PushClip>&) = 0;
		virtual void runCommand(Command<PopClip>&) = 0;
		virtual void runCommand(Command<ClearClip>&) = 0;
	};
}

#ifdef USE_OPENGL
#include "Guijo/Graphics/OpenGL.hpp"
#else
#error "No Graphics!"
#endif