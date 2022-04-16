LOAD_AS_STRING(
layout(location = 0) in vec2 aPos;

uniform mat4 mvp;
uniform vec4 dim;

out vec2 fragCoord;

void main() {
    gl_Position = mvp * vec4(aPos, 0.0, 1.0);
    fragCoord = vec2(aPos.x * dim.z, aPos.y * dim.w); // Coordinate in pixels
}
)