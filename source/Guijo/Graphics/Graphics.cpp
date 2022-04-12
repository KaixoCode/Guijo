#include "Guijo/Graphics/Graphics.hpp"
#include "Guijo/Utils/Pointer.hpp"

using namespace Guijo;

void GraphicsBase::render(DrawContext& context) {
    auto& commands = context.m_Commands;

    // Make sure clip is entire window at start
    m_Clip = { -1, -1, m_Size[0] + 2, m_Size[1] + 2 };
    Command<Clip> _clip{ m_Clip };
    runCommand(_clip);

    while (!commands.empty()) {
        runCommand(commands.front(),
            std::make_index_sequence<Commands::Amount>{});
        commands.pop();
    }
}

bool GraphicsBase::loadFont(const std::string& name) {
    using namespace std::string_literals;
    TCHAR szPath[MAX_PATH];
    SHGetFolderPathA(nullptr, CSIDL_FONTS, nullptr, 0, szPath);

    std::string _noExtension = szPath;
    _noExtension += "/"s + name;
    std::filesystem::path _font = _noExtension + ".ttf";

    if (std::filesystem::exists(_font)) {
        char* _name = new char[name.size() + 1];
        for (int i = 0; i < name.size(); i++)
            _name[i] = name[i];
        _name[name.size()] = '\0';
        m_Fonts.emplace(_name, Guijo::FontType{ _font.string() });
        return true;
    } else return false;
}

void GraphicsBase::loadFont(const std::string& path, const std::string& name) {
    m_Fonts.emplace(name, Guijo::FontType{ path });
}

void GraphicsBase::runCommand(Command<Translate>& v) {
    m_Matrix = glm::translate(m_Matrix, glm::vec3(v.translate[0], v.translate[1], 0));
    m_ViewProj = m_Projection * m_Matrix;
}

void GraphicsBase::scale(Vec2<float> v) {
    m_Matrix = glm::scale(m_Matrix, glm::vec3(v[0], v[1], 1.));
    m_ViewProj = m_Projection * m_Matrix;
}

void GraphicsBase::runCommand(Command<PushMatrix>&) {
    m_MatrixStack.push(m_Matrix);
}

void GraphicsBase::runCommand(Command<PopMatrix>&) {
    if (m_MatrixStack.size() > 1) {
        m_Matrix = m_MatrixStack.top();
        m_MatrixStack.pop();
        m_ViewProj = m_Projection * m_Matrix;
    }
}

void GraphicsBase::dimensions(const Dimensions<float>& dims) {
    m_Projection = glm::ortho(0.0f, std::max(dims.width(), 5.f), 0.0f, std::max(dims.height(), 5.f));
    m_ViewProj = m_Projection * m_Matrix;
    m_Size[0] = 2 / m_Projection[0][0];
    m_Size[1] = 2 / m_Projection[1][1];
}

float GraphicsBase::charWidth(const char c, const std::string_view& font, float size) {
    if (!m_Fonts.contains(font)) return 0;
    return (*m_Fonts.find(font)).second.Size(size).Char(c).advance >> 6;
}

float GraphicsBase::stringWidth(const std::string_view& c, const std::string_view& font, float size) {
    if (!m_Fonts.contains(font)) return 0;
    float _width = 0;
    auto& _font = (*m_Fonts.find(font)).second.Size(size);
    for (auto& _c : c)
        _width += _font.Char(_c).advance >> 6;
    return _width;
}