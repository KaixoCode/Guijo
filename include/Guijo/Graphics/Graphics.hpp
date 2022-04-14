#pragma once
#include "Guijo/Graphics/Context.hpp"
#include "Guijo/Graphics/Font.hpp"
#include "Guijo/Graphics/Shader.hpp"

namespace Guijo {
	class GraphicsBase {
	public:
		GraphicsBase() = default;
		GraphicsBase(GraphicsBase&&) = delete;
		GraphicsBase(const GraphicsBase&) = delete;

		void render();

		virtual void dimensions(const Dimensions<float>&);

		virtual void prepare() = 0; // Prepare for drawing (i.e. context switching)
		virtual void swapBuffers() = 0;

	protected:
		static std::map<std::string, Guijo::Font, std::less<>> Fonts;

		DrawContext context;

		std::stack<Dimensions<float>> clipStack;
		Dimensions<float> clip{};
		std::stack<glm::mat4> matrixStack;
		glm::mat4 matrix{ 1.0f };
		glm::mat4 projection{ 0.f };
		glm::mat4 viewProjection{ 0.f };

		Dimensions<float> windowSize{};
		float scaling = 1;
		Font* currentFont = nullptr;
		glm::vec4 fill{ 1, 1, 1, 1 };
		glm::vec4 stroke{ 1, 1, 1, 1 };
		float strokeWeight = 0;
		float fontSize = 16;
		Alignment textAlign = Align::Left | Align::Bottom;

		template<std::size_t ...Is> 
		void runCommand(CommandData& c, std::index_sequence<Is...>) {
			((c.type == Is ? (runCommand(c.get<Is>()), true) : false) || ...);
		}

		virtual void runCommand(Command<Fill>&);
		virtual void runCommand(Command<Stroke>&);
		virtual void runCommand(Command<StrokeWeight>&);
		virtual void runCommand(Command<Rect>&) = 0;
		virtual void runCommand(Command<Line>&) = 0;
		virtual void runCommand(Command<Circle>&) = 0;
		virtual void runCommand(Command<Triangle>&) = 0;
		virtual void runCommand(Command<Text>&) = 0;
		virtual void runCommand(Command<FontSize>&);
		virtual void runCommand(Command<SetFont>&);
		virtual void runCommand(Command<TextAlign>&);
		virtual void runCommand(Command<Translate>&);
		virtual void runCommand(Command<PushMatrix>&);
		virtual void runCommand(Command<PopMatrix>&);
		virtual void runCommand(Command<Viewport>&) = 0;
		virtual void runCommand(Command<Clip>&) = 0;
		virtual void runCommand(Command<PushClip>&) = 0;
		virtual void runCommand(Command<PopClip>&) = 0;
		virtual void runCommand(Command<ClearClip>&) = 0;

		friend class Font;
		friend class Window;
	};
}

#ifdef USE_OPENGL
#include "Guijo/Graphics/OpenGL.hpp"
#else
#error "No Graphics!"
#endif