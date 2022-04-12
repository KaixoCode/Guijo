#include "Guijo/Graphics/Graphics.hpp"

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
	auto& [dim, rotation, radius] = v;
	static const Shader _shader {
		// Vertex shader
		R"~~(
		#version 450 core
		layout(location = 0) in vec2 aPos;
		uniform mat4 mvp;
		void main() {
		    gl_Position = mvp * vec4(aPos.x, -aPos.y, 0.0, 1.0);
		}
		)~~",

		// Fragment shader
		R"~~(
		#version 450 core 
		out vec4 FragColor; 
		uniform vec4 color; 
		void main() { 
		    FragColor = color; 
		} 
		)~~"
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
		uniform vec4 radius;
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
		_shader.SetMat4(mvp, viewProjection * _model);
		_shader.SetVec4(color, fill);
	} else {
		if (m_PreviousShader != 5) {
			_shader2.Use();
			glBindVertexArray(quad.vao);
		}
		m_PreviousShader = 5;

		glm::vec4 _dim;
		_dim.x = (dim.x() + matrix[3].x) * projection[0].x + projection[3].x;
		_dim.y = (dim.y() + matrix[3].y) * projection[1].y + projection[3].y;
		_dim.z = dim.width() * projection[0].x;
		_dim.w = dim.height() * projection[1].y;

		glm::vec4 _mdim{ dim.x(), dim.y(), dim.width(), dim.height() };
		glm::vec4 _radi{ radius.x(), radius.y(), radius.width(), radius.height() };

		_shader2.SetVec4(dims2, _dim);
		_shader2.SetVec4(rdims2, _mdim);
		_shader2.SetVec4(radius2, _radi);
		_shader2.SetVec4(color2, fill);
	}

	glDrawArrays(GL_TRIANGLES, 0, 6);
}
void Graphics::runCommand(Command<Line>& v) {
	auto& [start, end, thickness] = v;

	static const Shader _shader {
		// Vertex shader
		R"~~(
		#version 330 core 
		layout(location = 0) in vec2 aPos; 
		uniform vec4 dim; 
		void main() { 
		    gl_Position = vec4(aPos.x * dim.z + dim.x, aPos.y * dim.w + dim.y, 0.0, 1.0); 
		}
		)~~",

		// Fragment shader
		R"~~(
		#version 330 core
		out vec4 FragColor; 
		uniform vec4 color; 
		uniform vec4 realdim; 
		uniform float width; 
		float minimum_distance(vec2 v, vec2 w, vec2 p) {
		    // Return minimum distance between line segment vw and point p
		    float l2 = pow(distance(w, v), 2);  // i.e. |w-v|^2 -  avoid a sqrt
		    if (l2 == 0.0) return distance(p, v);   // v == w case
		    // Consider the line extending the segment, parameterized as v + t (w - v).
		    // We find projection of point p onto the line.
		    // It falls where t = [(p-v) . (w-v)] / |w-v|^2
		    // We clamp t from [0,1] to handle points outside the segment vw.
		    float t = max(0, min(1, dot(p - v, w - v) / l2));
		    vec2 projection = v + t * (w - v);  // Projection falls on the segment
		    return distance(p, projection);
		}
		void main() { 
		    float dist = minimum_distance(realdim.zw, realdim.xy, gl_FragCoord.xy);
		    if (dist / width > 0.5) FragColor = vec4(color.rgb, 2 * (1 - (dist / width)) * color.a);
		    else FragColor = color;
		} 
		)~~"
	};
	static const GLint dims = glGetUniformLocation(_shader.ID, "dim");
	static const GLint realdim = glGetUniformLocation(_shader.ID, "realdim");
	static const GLint widths = glGetUniformLocation(_shader.ID, "width");
	static const GLint color = glGetUniformLocation(_shader.ID, "color");

	if (m_PreviousShader != 7) {
		_shader.Use();
		glBindVertexArray(line.vao);
	}
	m_PreviousShader = 7;

	thickness /= scaling;

	glm::vec4 dim = { 
		start.x(), start.y(), 
		end.x() - start.x(), 
		end.y() - start.y() 
	};

	glm::vec4 _tdim = dim;
	float delta_x = _tdim.z - _tdim.x;
	float delta_y = _tdim.w - _tdim.y;
	float angle = std::atan2(delta_y, delta_x);
	float dx = std::cos(angle);
	float dy = std::sin(angle);
	_tdim.x -= dx * thickness * 0.25;
	_tdim.z += dx * thickness * 0.25;
	_tdim.y -= dy * thickness * 0.25;
	_tdim.w += dy * thickness * 0.25;

	glm::vec4 _dim {
		(_tdim.x + matrix[3].x) * projection[0].x + projection[3].x,
		(_tdim.y + matrix[3].y) * projection[1].y + projection[3].y,
		(_tdim.z + matrix[3].x) * projection[0].x + projection[3].x - _dim.x,
		(_tdim.w + matrix[3].y) * projection[1].y + projection[3].y - _dim.y
	};

	glm::vec4 _rdim{
		(dim.x + matrix[3].x) / scaling,
		(dim.y + matrix[3].y) / scaling,
		(dim.z + matrix[3].x) / scaling,
		(dim.w + matrix[3].y) / scaling 
	};

	delta_x = _rdim.z - _rdim.x;
	delta_y = _rdim.w - _rdim.y;
	angle = std::atan2(delta_y, delta_x);
	dx = std::cos(angle);
	dy = std::sin(angle);
	_rdim.x += dx * thickness * 0.25;
	_rdim.z -= dx * thickness * 0.25;
	_rdim.y += dy * thickness * 0.25;
	_rdim.w -= dy * thickness * 0.25;

	_shader.SetVec4(dims, _dim);
	_shader.SetVec4(realdim, _rdim);
	_shader.SetFloat(widths, thickness * 0.5f);
	_shader.SetVec4(color, fill);

	glLineWidth(thickness);
	glDrawArrays(GL_LINES, 0, 2);
}

void Graphics::runCommand(Command<Ellipse>& v) {
	auto& [dim, a] = v;

	static const Shader _shader {
		R"~~(
		#version 330 core 
		
		layout(location = 0) in vec2 aPos; 
		
		uniform mat4 mvp; 
		
		void main() { 
		    gl_Position = mvp * vec4(aPos, 0.0, 1.0);
		} 
		)~~",

		R"~~(
		#version 330 core
		out vec4 FragColor;
		uniform vec4 color;
		uniform vec2 angles;
		uniform vec4 dimensions;
		void main() {
		    vec2 pos = gl_FragCoord.xy; 
		    float x = dimensions.x; 
		    float y = dimensions.y; 
		    float l = sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2)); 
		    float a = acos((pos.x - x)/l); 
		    if (y > pos.y) a = 6.28318530718-a; 
		    float astart = 0; 
		    float aend = angles.y - angles.x;
		    if (aend < 0) aend = aend + 6.28318530718; 
		    float aa = a - angles.x;
		    if (aa < 0) aa = aa + 6.28318530718; 
		    float r = (pow(pos.x - x, 2) / pow(dimensions.z / 2, 2)) + (pow(pos.y - y, 2) / pow(dimensions.w / 2, 2)); 
		    if (aa > aend) { discard; } 
		    else if (r > 1) { discard; } 
		    else if (r > 0.90) { FragColor = vec4(color.rgb, 10 * (1 - r) * color.a); } 
		    else { FragColor = color; } 
		} 
		)~~"
	};
	static const GLint mvp = glGetUniformLocation(_shader.ID, "mvp");
	static const GLint color = glGetUniformLocation(_shader.ID, "color");
	static const GLint angles = glGetUniformLocation(_shader.ID, "angles");
	static const GLint dimensions = glGetUniformLocation(_shader.ID, "dimensions");

	if (m_PreviousShader != 3) {
		_shader.Use();
		glBindVertexArray(ellipse.vao);
	}
	m_PreviousShader = 3;

	glm::mat4 _model{ 1.0f };
	_model = glm::translate(_model, glm::vec3{ dim.x(), dim.y(), 0});
	_model = glm::scale(_model, glm::vec3{ dim.width(), dim.height(), 1});

	_shader.SetMat4(mvp, viewProjection * _model);
	_shader.SetVec4(color, fill);

	constexpr auto PI = std::numbers::pi_v<double>;

	if (a.x() == 0 && a.y() == 0) 
		_shader.SetVec2(angles, { 0, PI * 2 });
	else {
		_shader.SetVec2(angles, {
			std::fmod(a.y() + 4.0 * PI, 2.0 * PI),
			std::fmod(a.x() + 4.0 * PI, 2.0 * PI)
		});
	}

	glm::vec4 _dim{ 
		(dim.x() + matrix[3].x) / scaling, 
		(dim.y() + matrix[3].y) / scaling, 
		dim.width() / scaling, 
		dim.height() / scaling
	};

	_shader.SetVec4(dimensions, _dim);

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

	if (m_PreviousShader != 2) {
		_shader.Use();
		glBindVertexArray(triangle.vao);
	}
	m_PreviousShader = 2;

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
	if (m_PreviousShader != 1) {
		_shader.Use();
		glBindVertexArray(text.vao);
	}
	m_PreviousShader = 1;

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
