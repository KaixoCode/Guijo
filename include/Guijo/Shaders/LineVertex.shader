LOAD_AS_STRING(
layout(location = 0) in vec2 aPos;
uniform vec4 dim;
void main() {
	gl_Position = vec4(aPos.x * dim.z + dim.x, aPos.y * dim.w + dim.y, 0.0, 1.0);
}
)