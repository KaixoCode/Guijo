#ifdef USE_OPENGL
#include "Guijo/Graphics/Graphics.hpp"
#define LOAD_AS_STRING(...) "#version 450 core \n"#__VA_ARGS__,

using namespace Guijo;

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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_SCISSOR_TEST);

	createBuffers();
}

void Graphics::createBuffers() {
	{	// Line
		constexpr float _vertices[] = {
			-.5f, -.5f,  .5f, -.5f,
			-.5f,  .5f,  .5f, -.5f,
			 .5f,  .5f, -.5f,  .5f,
		};

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
	if (m_Context != current) {
		wglMakeCurrent(m_Device, m_Context);
		current = m_Context;
	}

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
	}
	else {
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
		std::floor(v.viewport.height() / scaling));
}

void Graphics::runCommand(Command<Rect>& v) {
	auto& [dim, radius, rotation] = v;
	dim.y(windowSize.height() - dim.y() - dim.height()); // Flip y

	static const Shader _shader{
#include <Guijo/Shaders/RectVertex.shader>
#include <Guijo/Shaders/RectFragment.shader>
	};
	static const GLint uf_mvp = glGetUniformLocation(_shader.ID, "mvp");
	static const GLint uf_dim = glGetUniformLocation(_shader.ID, "dim");
	static const GLint uf_fillColor = glGetUniformLocation(_shader.ID, "fill");
	static const GLint uf_strokeColor = glGetUniformLocation(_shader.ID, "stroke");
	static const GLint uf_strokeWeight = glGetUniformLocation(_shader.ID, "strokeWeight");
	static const GLint uf_radius = glGetUniformLocation(_shader.ID, "radius");

	if (m_PreviousShader != Shaders::RectShader1) {
		_shader.Use();
		glBindVertexArray(quad.vao);
		m_PreviousShader = Shaders::RectShader1;
	}

	// Adjust 1 pixel for Anti-Aliasing.
	glm::vec4 _dim{ dim.x() - 1, dim.y() - 1, dim.width() + 2, dim.height() + 2 };
	glm::vec4 _radius{ radius.x(), radius.y(), radius.width(), radius.height(), };
	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ _dim.x, _dim.y, 0.f });
	if (rotation != 0) {
		_model = glm::translate(_model, glm::vec3{ _dim.z / 2, _dim.w / 2, 0. });
		_model = glm::rotate(_model, glm::radians(rotation), glm::vec3{ 0, 0, 1 });
		_model = glm::translate(_model, glm::vec3{ -_dim.z / 2, -_dim.w / 2, 0. });
	}
	_model = glm::scale(_model, glm::vec3{ _dim.z, _dim.w, 1 });

	_shader.SetMat4(uf_mvp, viewProjection * _model);
	_shader.SetVec4(uf_dim, _dim);
	_shader.SetVec4(uf_fillColor, fill);
	_shader.SetVec4(uf_strokeColor, stroke);
	_shader.SetFloat(uf_strokeWeight, strokeWeight);
	_shader.SetVec4(uf_radius, _radius);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Line>& v) {
	auto& [start, end, cap] = v;
	start.y(windowSize.height() - start.y()); // Flip y
	end.y(windowSize.height() - end.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/LineVertex.shader>
#include <Guijo/Shaders/LineFragment.shader>
	};
	static const GLint uf_mvp = glGetUniformLocation(_shader.ID, "mvp");
	static const GLint uf_length = glGetUniformLocation(_shader.ID, "length");
	static const GLint uf_type = glGetUniformLocation(_shader.ID, "type");
	static const GLint uf_color = glGetUniformLocation(_shader.ID, "color");

	if (m_PreviousShader != Shaders::LineShader) {
		_shader.Use();
		glBindVertexArray(line.vao);
		m_PreviousShader = Shaders::LineShader;
	}

	float thickness = strokeWeight / scaling;

	auto middle = start.to(end, 0.5);
	auto length = start.distance(end);
	
	float delta_x = end.x() - start.x();
	float delta_y = end.y() - start.y();
	float angle = std::atan2(delta_y, delta_x);

	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ middle.x(), middle.y(), 0.f});
	_model = glm::rotate(_model, angle, glm::vec3{ 0, 0, 1 });
	_model = glm::scale(_model, glm::vec3{ length + thickness, thickness + 0.5, 1 });

	_shader.SetMat4(uf_mvp, viewProjection * _model);
	_shader.SetVec2(uf_length, glm::vec2(length + thickness, thickness));
	_shader.SetVec4(uf_color, stroke);
	_shader.SetFloat(uf_type, static_cast<float>(cap));

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Circle>& v) {
	auto& [center, radius, angles] = v;
	center.y(windowSize.height() - center.y()); // Flip y

	static const Shader _shader {
#include <Guijo/Shaders/CircleVertex.shader>
#include <Guijo/Shaders/CircleFragment.shader>
	};
	static const GLint uf_mvp = glGetUniformLocation(_shader.ID, "mvp");
	static const GLint uf_dim = glGetUniformLocation(_shader.ID, "dim");
	static const GLint uf_fillColor = glGetUniformLocation(_shader.ID, "fill");
	static const GLint uf_strokeColor = glGetUniformLocation(_shader.ID, "stroke");
	static const GLint uf_strokeWeight = glGetUniformLocation(_shader.ID, "strokeWeight");
	static const GLint uf_angles = glGetUniformLocation(_shader.ID, "angles");

	if (m_PreviousShader != Shaders::EllipseShader) {
		_shader.Use();
		glBindVertexArray(ellipse.vao);
		m_PreviousShader = Shaders::EllipseShader;
	}

	glm::vec4 _dim{ center.x(), center.y(), 2 * radius + 2, 2 * radius + 2 };
	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ _dim.x, _dim.y, 0.f });
	_model = glm::scale(_model, glm::vec3{ _dim.z, _dim.w, 1 });

	_shader.SetMat4(uf_mvp, viewProjection * _model);
	_shader.SetVec4(uf_dim, _dim);
	_shader.SetVec4(uf_fillColor, fill);
	_shader.SetVec4(uf_strokeColor, stroke);
	_shader.SetFloat(uf_strokeWeight, strokeWeight);

	constexpr auto PI = std::numbers::pi_v<double>;

	if (angles.x() == 0 && angles.y() == 0)
		_shader.SetVec2(uf_angles, { 0, PI * 2 });
	else {
		_shader.SetVec2(uf_angles, {
			std::fmod(angles.y() + 4.0 * PI, 2.0 * PI),
			std::fmod(angles.x() + 4.0 * PI, 2.0 * PI)
		});
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void Graphics::runCommand(Command<Triangle>& v) {
	auto& [dim, rotation] = v;

	static const Shader _shader {
		R"~~(
		#version 330 core 
		layout(location = 0) in vec2 aPos; 
		uniform mat4 projection; 
		uniform mat4 view; 
		uniform mat4 model; 
		void main() { 
		    gl_Position = projection * view * model * vec4(aPos, 0.0, 1.0); 
		}
		)~~",
		R"~~(
		#version 330 core 
		out vec4 FragColor; 
		uniform vec4 color; 
		void main() { 
		    FragColor = color; 
		}
		)~~"
	};
	static const GLint model = glGetUniformLocation(_shader.ID, "model");
	static const GLint view = glGetUniformLocation(_shader.ID, "view");
	static const GLint proj = glGetUniformLocation(_shader.ID, "projection");
	static const GLint color = glGetUniformLocation(_shader.ID, "color");

	if (m_PreviousShader != Shaders::TriangleShader) {
		_shader.Use();
		glBindVertexArray(triangle.vao);
		m_PreviousShader = Shaders::TriangleShader;
	}

	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ dim.x(), dim.y() + dim.height(), 0});
	_model = glm::scale(_model, glm::vec3{ dim.width(), dim.height(), 1});
	if (rotation != 0) // Rotate only if necessary
		_model = glm::rotate(_model, glm::radians(rotation), glm::vec3{ 0, 0, 1 });

	_shader.SetMat4(model, _model);
	_shader.SetMat4(view, matrix);
	_shader.SetMat4(proj, projection);
	_shader.SetVec4(color, fill);

	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void Graphics::runCommand(Command<Text>& v) {
	auto& [str, pos] = v;

	static const Shader _shader {
		// Vertex shader
		R"~~(
		#version 330 core
		layout(location = 0) in vec2 aPos;
		uniform vec4 dim; 
		out vec2 texpos; 
		void main() { 
		    gl_Position = vec4(dim.x + aPos.x * dim.z, dim.y + aPos.y * dim.w, 0.0, 1.0); 
		    texpos = vec2(aPos.x, 1-aPos.y); 
		})~~",

		// Fragment shader
		R"~~(
		#version 330 core
	
		out vec4 col;
	
		uniform vec4 color;
		uniform sampler2DArray Texture;
	
		uniform int theTexture;
		in vec2 texpos;
	
		void main() {
		    vec3 sampled = texture(Texture, vec3(texpos.x, texpos.y, theTexture)).rgb;
		    col.a = (sampled.r + sampled.g + sampled.b) / 3;
		    col.r = sampled.r * color.r;
		    col.g = sampled.g * color.g;
		    col.b = sampled.b * color.b;
		})~~",
	};

	static const GLint color = glGetUniformLocation(_shader.ID, "color");
	static const GLint texture = glGetUniformLocation(_shader.ID, "Texture");
	static const GLint dims = glGetUniformLocation(_shader.ID, "dim");
	static const GLint theTexture = glGetUniformLocation(_shader.ID, "theTexture");

	if (!currentFont) return;

	glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
	if (m_PreviousShader != Shaders::TextShader) {
		_shader.Use();
		glBindVertexArray(text.vao);
		m_PreviousShader = Shaders::TextShader;
	}

	auto _charMap = &currentFont->size(std::round(fontSize));

	// Calculate the total width if we need it.
	float _totalWidth = 0.0f;
	if (textAlign & Align::Right || textAlign & Align::CenterX)
		for (int i = 0; i < str.size(); i++)
			_totalWidth += _charMap->character(str[i]).advance >> 6;
	
	// Bind the 3d texture for this charmap
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _charMap->texture);
	_shader.SetVec4(color, fill);
	_shader.SetInt(texture, 0); // We need to set the texture like this

	float _scale = fontSize / std::round(fontSize);
	float x = pos.x();
	float y = windowSize.height() - pos.y();

	// Alignment
	if (textAlign & Align::Middle) y -= _charMap->middle();
	else if (textAlign & Align::TextBottom) y -= _charMap->descender();
	else if (textAlign & Align::Baseline) y;
	else y -= _charMap->ascender() + _charMap->descender();
	if (textAlign & Align::CenterX) x -= 0.5 * _totalWidth * _scale;
	else if (textAlign & Align::Right) x -= _totalWidth * _scale;

	for (int i = 0; i < str.size(); i++) {
		char _c = str[i];

		auto& _ch = _charMap->character(_c);

		if (_c != ' ' && _c != '\f' && _c != '\r' 
			&& _c != '\t' && _c != '\v' && _c != '\n') {
			float _xpos = std::floor(x * matrix[0][0] + _ch.bearing.x() * _scale);
			float _ypos = std::floor(y - (_ch.size.height() - _ch.bearing.y()) * _scale);

			glm::vec4 _dim;
			_dim.x = (_xpos + matrix[3].x) * projection[0].x + projection[3].x;
			_dim.y = (_ypos + matrix[3].y) * projection[1].y + projection[3].y;
			_dim.z = fontSize * projection[0].x;
			_dim.w = fontSize * projection[1].y;

			_shader.SetVec4(dims, _dim);
			_shader.SetInt(theTexture, _c);

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		x += (_ch.advance >> 6) * _scale;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}
#endif