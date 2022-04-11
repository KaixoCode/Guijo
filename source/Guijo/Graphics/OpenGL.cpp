#include "Guijo/Graphics/Graphics.hpp"
#include <glad/glad.h>

using namespace Guijo;

//In an initialization routine
#ifdef WIN32
void Graphics::initialize(HDC hdc) {
	PIXELFORMATDESCRIPTOR _pfd{};
	_pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	_pfd.nVersion = 1;
	_pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType = PFD_TYPE_RGBA;
	_pfd.cColorBits = 32;
	_pfd.cDepthBits = 24;
	_pfd.cStencilBits = 8;
	_pfd.cAuxBuffers = 0;
	_pfd.iLayerType = PFD_MAIN_PLANE;
	auto iPixelFormat = ChoosePixelFormat(hdc, &_pfd);
	SetPixelFormat(hdc, iPixelFormat, &_pfd);
	m_Context = wglCreateContext(hdc);
	m_Device = hdc;
	auto res = wglMakeCurrent(m_Device, m_Context);
	
	constexpr auto ProcLoader = [](const char* name) -> void* {
		void* p = (void*)wglGetProcAddress(name);
		if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2)
			|| (p == (void*)0x3) || (p == (void*)-1)) {
			HMODULE module = LoadLibraryA("opengl32.dll");
			if (module == nullptr) return nullptr;
			else p = (void*)GetProcAddress(module, name);
		}
		return p;
	};
	// Static load, as we only need it once
	static auto loadGlad = gladLoadGLLoader((GLADloadproc)ProcLoader);
	if (!loadGlad) exit(-1);


}

void Graphics::createBuffers() {
	{	// Line
		constexpr float _vertices[] = { 0.0f, 0.0f, 1.0f, 1.0f, };

		glGenVertexArrays(1, &line.vao);
		glGenBuffers(1, &line.vbo);

		glBindVertexArray(line.vao);

		glBindBuffer(GL_ARRAY_BUFFER, line.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	{	// Quad
		float _vertices[] = {
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f,
		};

		glGenVertexArrays(1, &quad.vao);
		glGenBuffers(1, &quad.vbo);

		glBindVertexArray(quad.vao);

		glBindBuffer(GL_ARRAY_BUFFER, quad.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	{	// Ellipse
		float _vertices[] = {
			-.5f, -.5f,  .5f, -.5f,
			-.5f,  .5f,  .5f, -.5f,
			 .5f,  .5f, -.5f,  .5f,
		};

		glGenVertexArrays(1, &ellipse.vao);
		glGenBuffers(1, &ellipse.vbo);

		glBindVertexArray(ellipse.vao);

		glBindBuffer(GL_ARRAY_BUFFER, ellipse.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	{	// Triangle
		float _vertices[] = { 
			-0.5f, -0.5f, 
			 0.5f,  0.0f, 
			-0.5f,  0.5f 
		};

		glGenVertexArrays(1, &triangle.vao);
		glGenBuffers(1, &triangle.vbo);

		glBindVertexArray(triangle.vao);

		glBindBuffer(GL_ARRAY_BUFFER, triangle.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
	{	// Text
		float _vertices[] = {
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 1.0f, 1.0f, 0.0f,
			1.0f, 1.0f, 0.0f, 1.0f,
		};

		glGenVertexArrays(1, &text.vao);
		glGenBuffers(1, &text.vbo);

		glBindVertexArray(text.vao);

		glBindBuffer(GL_ARRAY_BUFFER, text.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(_vertices), _vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}
}


void Graphics::prepare() {
	wglMakeCurrent(m_Device, m_Context);
}
#else
void Graphics::prepare() {}
#endif

Graphics::~Graphics() {
	// Delete the context
	wglMakeCurrent(m_Device, nullptr);
	wglDeleteContext(m_Context);
}

void Graphics::runCommand(Command<Fill>& v) {
	m_Fill = v.color / 256;
}

void Graphics::runCommand(Command<Rect>&) {

}
void Graphics::runCommand(Command<Line>&) {}
void Graphics::runCommand(Command<Ellipse>&) {}
void Graphics::runCommand(Command<Triangle>&) {}
void Graphics::runCommand(Command<Text>&) {}
void Graphics::runCommand(Command<FontSize>&) {}
void Graphics::runCommand(Command<Font>&) {}
void Graphics::runCommand(Command<TextAlign>&) {}
void Graphics::runCommand(Command<LineHeight>&) {}
void Graphics::runCommand(Command<Translate>&) {}
void Graphics::runCommand(Command<PushMatrix>&) {}
void Graphics::runCommand(Command<PopMatrix>&) {}
void Graphics::runCommand(Command<Viewport>&) {}
void Graphics::runCommand(Command<Clip>&) {}
void Graphics::runCommand(Command<PushClip>&) {}
void Graphics::runCommand(Command<PopClip>&) {}
void Graphics::runCommand(Command<ClearClip>&) {}