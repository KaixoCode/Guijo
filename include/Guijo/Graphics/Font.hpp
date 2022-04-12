#pragma once
#include "Guijo/pch.hpp"
#include "Guijo/Utils/Vec.hpp"

namespace Guijo {
	class Font {
		static inline FT_Library library;
		static inline int references = 0;

		struct CharMap {
			struct Character {
				unsigned int index = static_cast<unsigned int>(-1);
				Point<int> size{};
				Point<int> bearing{};
				unsigned int advance{};
			};

			CharMap(int size, FT_Face& face);

			Character& character(char c);

			float height() const { return m_Ascender - m_Descender; }
			float ascender() const { return m_Ascender; }
			float descender() const { return m_Descender; }
			float middle() const { return (m_Size + m_Descender) / 2; }

			unsigned int texture;

		private:
			void initialize();

			Character m_CharMap[128]{};
			int m_Size{};
			float m_Ascender{};
			float m_Descender{};
			float m_Height{};
			FT_Face& m_Face;
		};

	public:
		static inline std::string_view Default = "segoeui";
		static void load(std::string_view path, std::string_view name);
		static bool load(std::string_view name);
		static float width(const char c, std::string_view font, float size);
		static float width(std::string_view c, std::string_view font, float size);

		Font(std::string_view path);
		Font(const Font& other);
		~Font();

		CharMap& size(int size);

	private:
		std::string m_Path{};
		FT_Face m_Face{};

		std::map<int, CharMap> m_SizeMap{};
	};
}