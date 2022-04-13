LOAD_AS_STRING(

out vec4 fragColor;
uniform vec4 color;
uniform vec2 length;
uniform float type;

in vec2 fragCoord;

float minimum_distance(vec2 v, vec2 w, vec2 p) {
	// Return minimum distance between line segment vw and point p
	float l2 = pow(distance(w, v), 2);  // i.e. |w-v|^2 -  avoid a sqrt
	if (l2 == 0.0) return distance(p, v);   // v == w case
	// Consider the line extending the segment, parameterized as v + t (w - v).
	// We find projection of point p onto the line.
	// It falls where t = [(p-v) . (w-v)] / |w-v|^2
	// We clamp t from [0,1] to handle points outside the segment vw.
	float t = max(0, min(1, dot(p - v, w - v) / l2));
	vec2 projection = v + t * (w - v);  // Projection falls on the segment
	return distance(p, projection);
}


void main() {
	float edgeSoftness = 0.5f;
	float width = length.y - edgeSoftness;
	float dist = 0;
	if (type == 0) {
		float len = length.x / 2.f - length.y / 2.f;
		dist = minimum_distance(vec2(-len, 0.f), vec2(len, 0.f), fragCoord.xy);
	} else if (type == 1) {
		float len = length.x / 2.f;
		dist = minimum_distance(vec2(-len, 0.f), vec2(len, 0.f), fragCoord.xy);
		float edgeDist = abs(fragCoord.x) - len + length.y / 2.f;
		if (edgeDist > 0.f) {
			dist = max(edgeDist, dist);
		}
	} else if (type == 2) {
		float len = length.x / 2.f - length.y / 2.f;
		dist = minimum_distance(vec2(-len, 0.f), vec2(len, 0.f), fragCoord.xy);
		float edgeDist = abs(fragCoord.x) - len + length.y / 2.f;
		if (edgeDist > 0.f) {
			dist = max(edgeDist, dist);
		}
	}
	float smoothedAlpha = smoothstep(-edgeSoftness, edgeSoftness, dist - width / 2.0f);
	vec4 fillColor = color;
	vec4 bgColor = vec4(fillColor.xyz, 0.0f);
	fragColor = mix(fillColor, bgColor, smoothedAlpha);
}
)