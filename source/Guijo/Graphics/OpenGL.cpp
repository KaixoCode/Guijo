#include "Guijo/Graphics/Graphics.hpp"

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

	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	//glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SCISSOR_TEST);

	createBuffers();
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

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);
}

void Graphics::swapBuffers() {
	wglSwapLayerBuffers(m_Device, WGL_SWAP_MAIN_PLANE);
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

void Graphics::runCommand(Command<Rect>& v) {
	auto [dim, rotation, radius] = v;
	static const Shader _shader {
		// Vertex shader
		"#version 450 core \n "
		"layout(location = 0) in vec2 aPos; "
		"uniform mat4 mvp; "
		"void main() { "
		"    gl_Position = mvp * vec4(aPos.x, -aPos.y, 0.0, 1.0); "
		"}",

		// Fragment shader
		"#version 450 core \n "
		"out vec4 FragColor; "
		"uniform vec4 color; "
		"void main() { "
		"    FragColor = color; "
		"} "

	};
	static const GLint mvp = glGetUniformLocation(_shader.ID, "mvp");
	static const GLint color = glGetUniformLocation(_shader.ID, "color");
	static const Shader _shader2 {
		// Vertex shader
		R"~~(
			#version 450 core
			layout(location = 0) in vec2 aPos;
			uniform vec4 dim;
			uniform vec4 rdim;
			out vec2 uv;
			out vec2 size;
			void main() {
				gl_Position = vec4(dim.x + aPos.x * dim.z, -(dim.y + aPos.y * dim.w), 0.0, 1.0);
				uv = vec2(aPos.x * rdim.z, aPos.y * rdim.w);
				size = vec2(rdim.z, rdim.w);
			}
		)~~",

		// Fragment shader
		R"~~(
			#version 450 core
			out vec4 FragColor;
			uniform vec4 color;
			uniform float radius;
			in vec2 uv;
			in vec2 size;
			float roundedFrame (float r, float thickness) {
				float res = 0;
				if (uv.x > r && uv.x < size.x - r
					  || uv.y > r && uv.y < size.y - r)
					res = 1;
				// bottom left
				else if (length(uv - vec2(r, r)) < r + thickness && uv.x < r && uv.y < r)
					res = 1 - (length(uv - vec2(r, r)) - r);
				// top left
				else if (length(uv - vec2(r, size.y - r)) < r + thickness && uv.x < r && uv.y > r)
					res = 1 - (length(uv - vec2(r, size.y - r)) - r);
				// bottom right
				else if (length(uv - vec2(size.x - r, r)) < r + thickness && uv.x > r && uv.y < r)
					res = 1 - (length(uv - vec2(size.x - r, r)) - r);
				// top right
				else if (length(uv - vec2(size.x - r, size.y - r)) < r + thickness && uv.x > r && uv.y > r)
					res = 1 - (length(uv - vec2(size.x - r, size.y - r)) - r);
    		
				return max(min(res, 1), 0);
			}
			void main() {    
				FragColor = vec4(color.rgb, color.a * roundedFrame(radius, 2));
			}
		)~~"
	};
	static const GLint dims2 = glGetUniformLocation(_shader2.ID, "dim");
	static const GLint rdims2 = glGetUniformLocation(_shader2.ID, "rdim");
	static const GLint radius2 = glGetUniformLocation(_shader2.ID, "radius");
	static const GLint color2 = glGetUniformLocation(_shader2.ID, "color");

	if (radius == 0 && rotation != 0) {
		if (m_PreviousShader != 6) {
			_shader.Use();
			glBindVertexArray(quad.vao);
		}
		m_PreviousShader = 6;

		glm::mat4 _model{ 1.0f };
		_model = glm::translate(_model, glm::vec3{ dim.x(), dim.y(), 0.f});
		_model = glm::translate(_model, glm::vec3{ dim.width() / 2, dim.height() / 2, 0.});
		_model = glm::rotate(_model, glm::radians(rotation), glm::vec3{ 0, 0, 1 });
		_model = glm::translate(_model, glm::vec3{ -dim.width() / 2, -dim.height() / 2, 0.});
		_model = glm::scale(_model, glm::vec3{ dim.width(), dim.height(), 1});
		_shader.SetMat4(mvp, m_ViewProj * _model);
		_shader.SetVec4(color, m_Fill);
	} else {
		if (m_PreviousShader != 5) {
			_shader2.Use();
			glBindVertexArray(quad.vao);
		}
		m_PreviousShader = 5;

		glm::vec4 _dim;
		_dim.x = (dim.x() + m_Matrix[3].x) * m_Projection[0].x + m_Projection[3].x;
		_dim.y = (dim.y() + m_Matrix[3].y) * m_Projection[1].y + m_Projection[3].y;
		_dim.z = dim.width() * m_Projection[0].x;
		_dim.w = dim.height() * m_Projection[1].y;

		glm::vec4 _mdim{ dim.x(), dim.y(), dim.width(), dim.height() };

		_shader2.SetVec4(dims2, _dim);
		_shader2.SetVec4(rdims2, _mdim);
		_shader2.SetFloat(radius2, std::min({ radius, dim.width() / 2, dim.height() / 2 }));
		_shader2.SetVec4(color2, m_Fill);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
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
void Graphics::runCommand(Command<Viewport>& v) {
	glViewport(
		std::floor(v.viewport.x() / m_Scaling),
		std::floor(v.viewport.y() / m_Scaling),
		std::floor(v.viewport.width() / m_Scaling),
		std::floor(v.viewport.height() / m_Scaling));
}

void Graphics::runCommand(Command<Clip>& v) {
	glEnable(GL_SCISSOR_TEST);
	Dimensions clip = {
		std::ceil((v.clip.x() + m_Matrix[3][0]) / m_Scaling),
			std::ceil((v.clip.y() + m_Matrix[3][1]) / m_Scaling),
			std::ceil(v.clip.width() / m_Scaling),
			std::ceil(v.clip.height() / m_Scaling)
	};
	clip = clip.overlaps(m_Clip);
	m_Clip = clip;
	glScissor(clip.x(), clip.y(), clip.width(), clip.height());
}

void Graphics::runCommand(Command<PushClip>&) {
	m_ClipStack.push(m_Clip);
}

void Graphics::runCommand(Command<PopClip>&) {
	if (m_ClipStack.size() == 0) {
		glDisable(GL_SCISSOR_TEST);
	} else {
		glEnable(GL_SCISSOR_TEST);
		Dimensions clip = m_ClipStack.top();
		m_ClipStack.pop();
		m_Clip = clip;
		glScissor(clip.x(), clip.y(), clip.width(), clip.height());
	}
}

void Graphics::runCommand(Command<ClearClip>&) {
	glDisable(GL_SCISSOR_TEST);
}