LOAD_AS_STRING(

out vec4 FragColor;
uniform vec4 color;
uniform vec4 realdim;
uniform float width;

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
	float dist = minimum_distance(realdim.zw, realdim.xy, gl_FragCoord.xy);
	if (dist / width > 0.5) FragColor = vec4(color.rgb, 2 * (1 - (dist / width)) * color.a);
	else FragColor = color;
}
)