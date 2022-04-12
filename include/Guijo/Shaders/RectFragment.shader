LOAD_AS_STRING(
out vec4 fragColor;

uniform vec4 fill;
uniform vec4 stroke;
uniform float strokeWeight;
uniform vec4 radius;

in vec2 fragCoord;
in vec2 size;

float roundedBoxSDF(vec2 CenterPosition, vec2 Size, vec4 Radius) {
	Radius.xy = (CenterPosition.x > 0.0) ? Radius.xy : Radius.zw;
	Radius.x = (CenterPosition.y > 0.0) ? Radius.x : Radius.y;
	vec2 q = abs(CenterPosition) - Size + Radius.x;
	return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - Radius.x;
}

void main() {
	// How soft the edges should be (in pixels). Higher values could be used to simulate a drop shadow.
	float edgeSoftness = 1.0f;
	vec2 theSize = vec2(size.x - 2.0 - edgeSoftness, size.y - 2.0 - edgeSoftness);
	// The pixel space location of the rectangle.
	vec2 location = vec2(round(size.x / 2.0), round(size.y / 2.0));
	// Calculate distance to edge.   
	float distance = roundedBoxSDF(fragCoord.xy - location, theSize / 2.0f, radius);
	// Smooth the result (free antialiasing).
	float smoothedAlpha = 1.0f - smoothstep(0.0f, edgeSoftness, distance);
	// Border.  
	float borderSoftness = 1.0f;
	float borderAlpha = 1.0f - smoothstep(strokeWeight - borderSoftness, strokeWeight, abs(distance));
	vec4 bgColor = vec4(stroke.xyz, 0.0f);
	if (strokeWeight == 0) bgColor = vec4(fill.xyz, 0.0f);
	vec4 fillColor = fill;
	// When fill color alpha is 0, set to stroke color for anti-aliasing, so there's no color blending
	if (fill.w == 0) fillColor = vec4(stroke.xyz, 0.0f);
	fragColor = mix(bgColor, mix(fillColor, stroke, borderAlpha), smoothedAlpha);
}
)