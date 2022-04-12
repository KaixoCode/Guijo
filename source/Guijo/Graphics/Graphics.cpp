#include "Guijo/Graphics/Graphics.hpp"
#include "Guijo/Utils/Pointer.hpp"

using namespace Guijo;

std::map<std::string, Guijo::Font, std::less<>> GraphicsBase::Fonts{};

void GraphicsBase::render() {
    auto& commands = context.m_Commands;

    // Make sure clip is entire window at start
    clip = { -1, -1, windowSize.width() + 2, windowSize.height() + 2};
    Command<Clip> _clip{ clip };
    runCommand(_clip);

    while (!commands.empty()) {
        runCommand(commands.front(),
            std::make_index_sequence<Commands::Amount>{});
        commands.pop();
    }
}

void GraphicsBase::dimensions(const Dimensions<float>& dims) {
    projection = glm::ortho(0.0f, std::max(dims.width(), 5.f), 0.0f, std::max(dims.height(), 5.f));
    viewProjection = projection * matrix;
    windowSize = dims;
}

void GraphicsBase::runCommand(Command<Fill>& v) {
    fill = v.color / 256;
}

void GraphicsBase::runCommand(Command<FontSize>& v) {
    fontSize = v.size;
}

void GraphicsBase::runCommand(Command<SetFont>& v) {
    auto _it = Fonts.find(v.font);
    if (_it != Fonts.end())
        currentFont = &_it->second;
    else {
        if (Font::load(v.font))
            currentFont = &Fonts.find(v.font)->second;
    }
}

void GraphicsBase::runCommand(Command<TextAlign>& v) {
    textAlign = v.align;
}

void GraphicsBase::runCommand(Command<Translate>& v) {
    matrix = glm::translate(matrix, glm::vec3(v.translate.x(), v.translate.y(), 0));
    viewProjection = projection * matrix;
}

void GraphicsBase::runCommand(Command<PushMatrix>&) {
    matrixStack.push(matrix);
}

void GraphicsBase::runCommand(Command<PopMatrix>&) {
    if (matrixStack.size() > 1) {
        matrix = matrixStack.top();
        matrixStack.pop();
        viewProjection = projection * matrix;
    }
}
