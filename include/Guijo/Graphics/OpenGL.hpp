#include "Guijo/pch.hpp"
#include "Guijo/Graphics/Context.hpp"

namespace Guijo {
	class Graphics : public GraphicsBase {
		friend class Window;
	public:		
		void runCommand(Command<Rect>&) override;
		void runCommand(Command<Line>&) override;
		void runCommand(Command<Circle>&) override;
		void runCommand(Command<Triangle>&) override;
		void runCommand(Command<Text>&) override;
		void runCommand(Command<Viewport>&) override;
		void runCommand(Command<Clip>&) override;
		void runCommand(Command<PushClip>&) override;
		void runCommand(Command<PopClip>&) override;
		void runCommand(Command<ClearClip>&) override;

		~Graphics();
	private:
		static inline Graphics* mainContext = nullptr;
		static inline HGLRC current = nullptr;
	    HGLRC m_Context = nullptr;
		static inline std::mutex m_Lock;

#ifdef WIN32
		HDC m_Device = nullptr;
		void initialize(HDC handle);
#endif
		void prepare() override; // Prepare for drawing (i.e. context switching)
		void swapBuffers() override;

		void createBuffers();

		struct Buffer {
			unsigned int vao;
			unsigned int vbo;

			void bind() const;
		};

		Buffer rect;
		Buffer textured;
		Buffer line;
		Buffer circle;
		Buffer triangle;
		Buffer text;
	};
}