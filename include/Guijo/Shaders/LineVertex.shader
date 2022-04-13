LOAD_AS_STRING(
layout(location = 0) in vec2 aPos;
uniform mat4 mvp;
uniform vec2 length;

out vec2 fragCoord;

void main() {
	gl_Position = mvp * vec4(aPos.x, aPos.y, 0.0, 1.0);
	fragCoord = vec2(aPos.x * length.x, aPos.y * length.y); // Coordinate in pixels
}
)