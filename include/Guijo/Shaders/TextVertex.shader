LOAD_AS_STRING(
layout(location = 0) in vec2 aPos;
uniform vec4 dim;
out vec2 texturePosition;
void main() {
	gl_Position = vec4(dim.x + aPos.x * dim.z, dim.y + aPos.y * dim.w, 0.0, 1.0);
	texturePosition = vec2(aPos.x, 1 - aPos.y);
}
)