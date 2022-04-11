#include "Guijo/Graphics/Context.hpp"

namespace Guijo {
	class GraphicsBase {
	public:
		GraphicsBase() = default;
		GraphicsBase(GraphicsBase&&) = delete;
		GraphicsBase(const GraphicsBase&) = delete;

		void render(DrawContext& context) {
			auto& commands = context.m_Commands;

			while (!commands.empty()) {
				runCommand(commands.front(),
					std::make_index_sequence<Commands::Amount>{});
				commands.pop();
			}
		}

	private:
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