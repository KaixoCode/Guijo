#ifdef USE_OPENGL
#include "Guijo/Graphics/Graphics.hpp"
#define LOAD_AS_STRING(...) "#version 450 core \n"#__VA_ARGS__,

using namespace Guijo;

#ifdef WIN32
void Graphics::initialize(HDC hdc) {
	static PIXELFORMATDESCRIPTOR _pfd{
		.nSize = sizeof(PIXELFORMATDESCRIPTOR),
		.nVersion = 1,
		.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		.iPixelType = PFD_TYPE_RGBA,
		.cColorBits = 32,
		.cDepthBits = 24,
		.cStencilBits = 8,
		.cAuxBuffers = 0,
		.iLayerType = PFD_MAIN_PLANE,
	};
	auto iPixelFormat = ChoosePixelFormat(hdc, &_pfd);
	SetPixelFormat(hdc, iPixelFormat, &_pfd);

	m_Device = hdc;
	if (mainContext == nullptr) { // mark first created context as main
		m_Context = wglCreateContext(hdc); // so we can link other contexts
		mainContext = this;
	} else {
		wglMakeCurrent(mainContext->m_Device, mainContext->m_Context);
		m_Context = wglCreateContext(hdc);
		wglShareLists(mainContext->m_Context, m_Context);
		constexpr bool useSwap = false;
		if constexpr (useSwap) {
			typedef BOOL(WINAPI* PFNWGLSWAPINTERVALEXTPROC)(int);
			PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)
				wglGetProcAddress("wglSwapIntervalEXT");
			wglMakeCurrent(m_Device, m_Context);
			wglSwapIntervalEXT(0);
		}
	}

	wglMakeCurrent(m_Device, m_Context);

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_SCISSOR_TEST);

	createBuffers();

	wglMakeCurrent(NULL, NULL);
}

void Graphics::Buffer::bind() const {
	glBindVertexArray(vao);
}

void Graphics::createBuffers() {
	constexpr auto generate = [](auto& vertices, auto& shape) {
		glGenVertexArrays(1, &shape.vao);
		glGenBuffers(1, &shape.vbo);
		glBindVertexArray(shape.vao);
		glBindBuffer(GL_ARRAY_BUFFER, shape.vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	};

	constexpr float _centered[] {
		-.5f, -.5f,  .5f, -.5f,
		-.5f,  .5f,  .5f, -.5f,
		 .5f,  .5f, -.5f,  .5f,
	};

	constexpr float _cornered[] {
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 1.0f, 0.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
	};

	generate(_centered, line);
	generate(_centered, circle);
	generate(_cornered, rect);
	generate(_cornered, triangle);
	generate(_cornered, text);
}

void Graphics::prepare() {
	if (m_Context != current) {
		auto v = wglMakeCurrent(m_Device, m_Context);
		current = m_Context;
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0, 0, 0, 0);
}

void Graphics::swapBuffers() {
	auto res = wglSwapLayerBuffers(m_Device, WGL_SWAP_MAIN_PLANE);
	int oaine = res;
}
#else
void Graphics::prepare() {}
#endif

Graphics::~Graphics() {
	if (m_Context == current)
		wglMakeCurrent(m_Device, nullptr);
	wglDeleteContext(m_Context);
}

void Graphics::runCommand(Command<Clip>& v) {
	glEnable(GL_SCISSOR_TEST);
	Dimensions<float> _clip = {
		std::ceil((v.clip.x() + matrix[3][0]) / scaling),
		std::ceil((v.clip.y() + matrix[3][1]) / scaling),
		std::ceil(v.clip.width() / scaling),
		std::ceil(v.clip.height() / scaling)
	};
	_clip = _clip.overlap(clip);
	clip = _clip;
	glScissor(clip.x(), clip.y(), clip.width(), clip.height());
}

void Graphics::runCommand(Command<PushClip>&) {
	clipStack.push(clip);
}

void Graphics::runCommand(Command<PopClip>&) {
	if (clipStack.size() == 0) {
		glDisable(GL_SCISSOR_TEST);
	} else {
		glEnable(GL_SCISSOR_TEST);
		Dimensions clip = clipStack.top();
		clipStack.pop();
		clip = clip;
		glScissor(clip.x(), clip.y(), clip.width(), clip.height());
	}
}

void Graphics::runCommand(Command<ClearClip>&) {
	glDisable(GL_SCISSOR_TEST);
}

void Graphics::runCommand(Command<Viewport>& v) {
	glViewport(
		std::floor(v.viewport.x() / scaling),
		std::floor(v.viewport.y() / scaling),
		std::floor(v.viewport.width() / scaling),
		std::floor(v.viewport.height() / scaling)
	);
}

void Graphics::runCommand(Command<Rect>& v) {
	auto& [dim, radius, rotation] = v;
	dim.y(windowSize.height() - dim.y() - dim.height()); // Flip y

	static const Shader _shader{
#include <Guijo/Shaders/RectVertex.shader>
#include <Guijo/Shaders/RectFragment.shader>
	};
	static const GLint uf_mvp = _shader.uniform("mvp");
	static const GLint uf_dim = _shader.uniform("dim");
	static const GLint uf_fillColor = _shader.uniform("fill");
	static const GLint uf_strokeColor = _shader.uniform("stroke");
	static const GLint uf_strokeWeight = _shader.uniform("strokeWeight");
	static const GLint uf_radius = _shader.uniform("radius");

	if (_shader.use()) rect.bind();

	// Adjust 1 pixel for Anti-Aliasing.
	glm::vec4 _dim{ dim.x() - 1, dim.y() - 1, dim.width() + 2, dim.height() + 2 };
	glm::vec4 _radius{ radius.x(), radius.y(), radius.width(), radius.height(), };
	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ _dim.x, _dim.y, 0.f });
	if (rotation.radians() != 0) {
		_model = glm::translate(_model, glm::vec3{ _dim.z / 2, _dim.w / 2, 0. });
		_model = glm::rotate(_model, rotation.radians(), glm::vec3{0, 0, 1});
		_model = glm::translate(_model, glm::vec3{ -_dim.z / 2, -_dim.w / 2, 0. });
	}
	_model = glm::scale(_model, glm::vec3{ _dim.z, _dim.w, 1 });

	_shader[uf_mvp] = viewProjection * _model;
	_shader[uf_dim] = _dim;
	_shader[uf_fillColor] = fill;
	_shader[uf_strokeColor] = stroke;
	_shader[uf_strokeWeight] = strokeWeight;
	_shader[uf_radius] = _radius;

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Line>& v) {
	// Line is basically drawn as a rotated rectangle, with
	// special cases for both ends.
	auto& [start, end, cap] = v;
	start.y(windowSize.height() - start.y()); // Flip y
	end.y(windowSize.height() - end.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/LineVertex.shader>
#include <Guijo/Shaders/LineFragment.shader>
	};
	static const GLint uf_mvp = _shader.uniform("mvp");
	static const GLint uf_length = _shader.uniform("length");
	static const GLint uf_type = _shader.uniform("type");
	static const GLint uf_color = _shader.uniform("color");

	if (_shader.use()) line.bind();

	const float thickness = strokeWeight / scaling;

	const auto middle = start.to(end, 0.5);
	const auto length = start.distance(end);
	
	const float delta_x = end.x() - start.x();
	const float delta_y = end.y() - start.y();
	const float angle = std::atan2(delta_y, delta_x);

	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ middle.x(), middle.y(), 0.f});
	_model = glm::rotate(_model, angle, glm::vec3{ 0, 0, 1 });
	_model = glm::scale(_model, glm::vec3{ length + thickness, thickness + 0.5, 1 });

	_shader[uf_mvp] = viewProjection * _model;
	_shader[uf_length] = glm::vec2(length + thickness, thickness);
	_shader[uf_color] = stroke;
	_shader[uf_type] = static_cast<int>(cap);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Circle>& v) {
	auto& [center, radius, angles] = v;
	center.y(windowSize.height() - center.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/CircleVertex.shader>
#include <Guijo/Shaders/CircleFragment.shader>
	};
	static const GLint uf_mvp = _shader.uniform("mvp");
	static const GLint uf_dim = _shader.uniform("dim");
	static const GLint uf_fillColor = _shader.uniform("fill");
	static const GLint uf_strokeColor = _shader.uniform("stroke");
	static const GLint uf_strokeWeight = _shader.uniform("strokeWeight");
	static const GLint uf_angles = _shader.uniform("angles");

	if (_shader.use()) circle.bind();

	glm::vec4 _dim{ center.x(), center.y(), 2 * radius + 2, 2 * radius + 2 };
	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ _dim.x, _dim.y, 0.f });
	_model = glm::scale(_model, glm::vec3{ _dim.z, _dim.w, 1 });

	_shader[uf_mvp] = viewProjection * _model;
	_shader[uf_dim] = _dim;
	_shader[uf_fillColor] = fill;
	_shader[uf_strokeColor] = stroke;
	_shader[uf_strokeWeight] = strokeWeight;
	_shader[uf_angles] = { angles[1].normalized(), angles[0].normalized() };
	
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Triangle>& v) {
	auto& [a, b, c] = v;
	a.y(windowSize.height() - a.y()); // Flip y
	b.y(windowSize.height() - b.y()); // Flip y
	c.y(windowSize.height() - c.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/TriangleVertex.shader>
#include <Guijo/Shaders/TriangleFragment.shader>
	};
	static const GLint uf_mvp = _shader.uniform("mvp");
	static const GLint uf_size = _shader.uniform("size");
	static const GLint uf_a = _shader.uniform("a");
	static const GLint uf_b = _shader.uniform("b");
	static const GLint uf_c = _shader.uniform("c");
	static const GLint uf_fill = _shader.uniform("fill");
	static const GLint uf_stroke = _shader.uniform("stroke");
	static const GLint uf_strokeWeight = _shader.uniform("strokeWeight");

	if (_shader.use()) triangle.bind();

	Point<float> _min{
		std::min({ a.x(), b.x(), c.x() }),
		std::min({ a.y(), b.y(), c.y() })
	};

	Point<float> _max{
		std::max({ a.x(), b.x(), c.x() }),
		std::max({ a.y(), b.y(), c.y() })
	};

	glm::vec4 _dim{ // Calculate the rectangle that fits the triangle
		_min.x(), _min.y(),
		_max.x() - _min.x(),
		_max.y() - _min.y(),
	};

	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ _dim.x, _dim.y, 0.f});
	_model = glm::scale(_model, glm::vec3{ _dim.z, _dim.w, 1 });

	glm::vec2 _a{ a.x() - _min.x(), a.y() - _min.y() };
	glm::vec2 _b{ b.x() - _min.x(), b.y() - _min.y() };
	glm::vec2 _c{ c.x() - _min.x(), c.y() - _min.y() };

	_shader[uf_mvp] = viewProjection * _model;
	_shader[uf_size] = glm::vec2(_dim.z, _dim.w);
	_shader[uf_fill] = fill;
	_shader[uf_stroke] = stroke;
	_shader[uf_strokeWeight] = strokeWeight;
	_shader[uf_a] = _a;
	_shader[uf_b] = _b;
	_shader[uf_c] = _c;

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Text>& v) {
	auto& [str, pos] = v;
	pos.y(windowSize.height() - pos.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/TextVertex.shader>
#include <Guijo/Shaders/TextFragment.shader>
	};

	static const GLint uf_color = _shader.uniform("color");
	static const GLint uf_fontmap = _shader.uniform("fontmap");
	static const GLint uf_character = _shader.uniform("character");
	static const GLint uf_dim = _shader.uniform("dim");

	// No font selected, so can't render text
	if (!currentFont) return;

	// For text rendering we use a different blend function
	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if (_shader.use()) text.bind();

	// Get the character map from the current font
	auto& _charMap = currentFont->size(std::round(fontSize));

	// Calculate the total width if we need it.
	float _totalWidth = 0.0f;
	if (textAlign & Align::Right || textAlign & Align::CenterX)
		for (int i = 0; i < str.size(); i++)
			_totalWidth += _charMap.character(str[i]).advance >> 6;
	
	// Bind the 3d texture for this charmap
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _charMap.texture);
	_shader[uf_color] = fill;
	_shader[uf_fontmap] = 0; // We need to set the texture like this

	// Adjust for non-integer size
	float _scale = fontSize / std::round(fontSize); 
	
	// Adjust position with vertical alignment
	if (textAlign & Align::Middle) pos.y(pos.y() - _charMap.middle());
	else if (textAlign & Align::TextBottom) pos.y(pos.y() - _charMap.descender());
	else if (textAlign & Align::Baseline);
	else pos.y(pos.y() - (_charMap.ascender() + _charMap.descender()));

	// Adjust position with horizontal alignment
	if (textAlign & Align::CenterX) pos.x(pos.x() - 0.5 * _totalWidth * _scale);
	else if (textAlign & Align::Right) pos.x(pos.x() - _totalWidth * _scale);

	// Draw all the characters
	for (char _c : str) {
		auto& _ch = _charMap.character(_c);

		// Some characters that shouldn't be drawn
		constexpr static auto blacklist = [](char _c) {
			return _c == ' '  || _c == '\f' || _c == '\r'
				|| _c == '\t' || _c == '\v' || _c == '\n';
		};

		if (!blacklist(_c)) { // If character not in blacklist, draw it
			float _xpos = std::floor(pos.x() * matrix[0][0] + _ch.bearing.x() * _scale);
			float _ypos = std::floor(pos.y() - (_ch.size.height() - _ch.bearing.y()) * _scale);

			glm::vec4 _dim{};
			_dim.x = (_xpos + matrix[3].x) * projection[0].x + projection[3].x;
			_dim.y = (_ypos + matrix[3].y) * projection[1].y + projection[3].y;
			_dim.z = fontSize * projection[0].x;
			_dim.w = fontSize * projection[1].y;

			_shader[uf_dim] = _dim;
			_shader[uf_character] = _c;

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		pos.x(pos.x() + (_ch.advance >> 6) * _scale);
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif