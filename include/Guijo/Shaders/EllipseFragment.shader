LOAD_AS_STRING(
out vec4 FragColor;
uniform vec4 color;
uniform vec2 angles;
uniform vec4 dimensions;
void main() {
	vec2 pos = gl_FragCoord.xy;
	float x = dimensions.x;
	float y = dimensions.y;
	float l = sqrt(pow(x - pos.x, 2) + pow(y - pos.y, 2));
	float a = acos((pos.x - x) / l);
	if (y > pos.y) a = 6.28318530718 - a;
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
)