#include "Guijo/pch.hpp"
#include "Guijo/Graphics/Context.hpp"

namespace Guijo {
	class Graphics : public GraphicsBase {
		friend class Window;
	public:		
		void runCommand(Command<Fill>&) override;
		void runCommand(Command<Rect>&) override;
		void runCommand(Command<Line>&) override;
		void runCommand(Command<Ellipse>&) override;
		void runCommand(Command<Triangle>&) override;
		void runCommand(Command<Text>&) override;
		void runCommand(Command<FontSize>&) override;
		void runCommand(Command<Font>&) override;
		void runCommand(Command<TextAlign>&) override;
		void runCommand(Command<LineHeight>&) override;
		void runCommand(Command<Translate>&) override;
		void runCommand(Command<PushMatrix>&) override;
		void runCommand(Command<PopMatrix>&) override;
		void runCommand(Command<Viewport>&) override;
		void runCommand(Command<Clip>&) override;
		void runCommand(Command<PushClip>&) override;
		void runCommand(Command<PopClip>&) override;
		void runCommand(Command<ClearClip>&) override;

		~Graphics();
	private:
	    HGLRC m_Context = nullptr;
		
		int m_PreviousShader = -1;
		Color m_Fill{ 1, 1, 1, 1 };
		float m_FontSize = 16;
		float m_LineHeight = 1.2;
		Alignment m_TextAlign = Align::Left | Align::Bottom;

		//GuiCode::Font* m_CurrentFont = nullptr;

#ifdef WIN32
		HDC m_Device = nullptr;
		void initialize(HDC handle);
#endif
		void prepare() override; // Prepare for drawing (i.e. context switching)
		void swapBuffers() override;

		void createBuffers();

		struct { unsigned int vao, vbo; } quad;
		struct { unsigned int vao, vbo; } textured;
		struct { unsigned int vao, vbo; } line;
		struct { unsigned int vao, vbo; } ellipse;
		struct { unsigned int vao, vbo; } triangle;
		struct { unsigned int vao, vbo; } text;
	};
}