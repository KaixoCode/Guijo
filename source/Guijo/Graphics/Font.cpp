#include "Guijo/Graphics/Font.hpp"
#include "Guijo/Graphics/Graphics.hpp"

#define CHECK(x, msg, then) if (auto error = x) { std::cout << msg << '\n'; then; }

using namespace Guijo;

Font::CharMap::CharMap(int size, FT_Face& face) 
	: m_Size(size), m_Face(face) { initialize(); }

Font::CharMap::Character& Font::CharMap::character(char c) {
	if (c < 0) return m_CharMap[0];
	else return m_CharMap[c];
}

void Font::CharMap::initialize() {
	FT_Set_Pixel_Sizes(m_Face, 0, m_Size);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	int _width = m_Size;
	int _height = m_Size;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, _width, _height, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	m_Ascender = m_Face->size->metrics.ascender / 64.f;
	m_Descender = m_Face->size->metrics.descender / 64.f;
	m_Height = m_Face->size->metrics.height / 64.f;

	unsigned char* empty = new unsigned char[_width * _height * 4];
	for (int i = 0; i < _width * _height * 4; i++)
		empty[i] = 0;

	for (unsigned int _c = 0; _c < 128; _c++) {
		if (FT_Load_Char(m_Face, _c, FT_LOAD_DEFAULT)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph\n";
			continue;
		}

		if (FT_Render_Glyph(m_Face->glyph, FT_RENDER_MODE_LCD)) {
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph\n";
			continue;
		}

		Character _character = {
			_c,
			{ m_Face->glyph->bitmap.width / 3, m_Face->glyph->bitmap.rows },
			{ m_Face->glyph->bitmap_left, m_Face->glyph->bitmap_top },
			static_cast<unsigned int>(m_Face->glyph->advance.x)
		};

		// Coords of small array
		int subx = 0;
		int suby = -(_height - _character.size.height()); // Initial offset for y
		for (int y = 0; y < _height; y++) {
			for (int x = 0; x < _width * 4; x++) {
				// Default value = 0
				unsigned char value = 0;

				// Don't get alpha values, because they don't exist
				bool notAlpha = (x % 4) != 3;

				// Make sure subx and suby within boundaries
				if (notAlpha && subx < m_Face->glyph->bitmap.width &&
					suby >= 0 && suby < m_Face->glyph->bitmap.rows) {
					// Sub index using amount of bytes * y + x
					int subindex = suby * m_Face->glyph->bitmap.pitch + subx;
					value = m_Face->glyph->bitmap.buffer[subindex];
					subx++; // Increment the x
				}

				// Index in bigger array
				int index = y * (_width * 4) + x;
				empty[index] = value;
			}
			subx = 0; // reset x
			suby++;   // increment y
		}

		glTexSubImage3D(
			GL_TEXTURE_2D_ARRAY,
			0, 0, 0, _c,
			_width,
			_height,
			1, GL_RGBA,
			GL_UNSIGNED_BYTE,
			empty
		);

		m_CharMap[_c] = _character;
	}
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
	delete[] empty;
}

Font::Font(std::string_view path) : m_Path(path) {
	++references;
	if (!library)
		CHECK(FT_Init_FreeType(&library), "Failed to initialize FreeType2 library", return);

	CHECK(FT_New_Face(library, m_Path.c_str(), 0, &m_Face), "Failed to open font file " << path, return);
}

Font::Font(const Font& other) : m_Path(other.m_Path) {
	++references;
	CHECK(FT_New_Face(library, m_Path.c_str(), 0, &m_Face), "Failed to open font file " << m_Path, return);
}

Font::~Font() {
	CHECK(FT_Done_Face(m_Face), "Failed to free font face", ;);

	if (--references == 0)
		CHECK(FT_Done_FreeType(library), "Failed to free FreeType2 library", return);
}

Font::CharMap& Font::size(int size) {
	return m_SizeMap.contains(size)
		? m_SizeMap.find(size)->second
		: m_SizeMap.insert({ size, { size, m_Face } }).first->second;
}

bool Font::load(std::string_view name) {
	using namespace std::string_literals;
	TCHAR szPath[MAX_PATH];
	SHGetFolderPathA(nullptr, CSIDL_FONTS, nullptr, 0, szPath);

	std::string _noExtension = szPath;
	_noExtension += "/"s + name.data();
	std::filesystem::path _font = _noExtension + ".ttf";

	if (std::filesystem::exists(_font)) {
		char* _name = new char[name.size() + 1];
		for (int i = 0; i < name.size(); i++)
			_name[i] = name[i];
		_name[name.size()] = '\0';
		GraphicsBase::Fonts.emplace(_name, Guijo::Font{ _font.string() });
		return true;
	} else return false;
}

void Font::load(std::string_view path, std::string_view name) {
	GraphicsBase::Fonts.emplace(name, Guijo::Font{ path });
}

float Font::width(const char c, std::string_view font, float size) {
	if (!GraphicsBase::Fonts.contains(font)) return 0;
	return (*GraphicsBase::Fonts.find(font)).second.size(size).character(c).advance >> 6;
}

float Font::width(std::string_view c, std::string_view font, float size) {
	if (!GraphicsBase::Fonts.contains(font)) return 0;
	float _width = 0;
	auto& _font = (*GraphicsBase::Fonts.find(font)).second.size(size);
	for (auto& _c : c)
		_width += _font.character(_c).advance >> 6;
	return _width;
}