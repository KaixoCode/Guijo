LOAD_AS_STRING(
out vec4 fragColor;

uniform vec4 color;
uniform sampler2DArray fontmap;

uniform int character;
in vec2 texturePosition;

void main() {
	vec3 sampled = texture(fontmap, vec3(texturePosition.x, texturePosition.y, character)).rgb;
	fragColor.a = (sampled.r + sampled.g + sampled.b) / 3;
	fragColor.r = sampled.r * color.r;
	fragColor.g = sampled.g * color.g;
	fragColor.b = sampled.b * color.b;
}
)