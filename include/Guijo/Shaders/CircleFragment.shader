LOAD_AS_STRING(
out vec4 fragColor;

uniform vec4 fill;
uniform vec4 stroke;
uniform float strokeWeight;
uniform vec2 angles;
uniform vec4 dim;

in vec2 fragCoord;

float distance_point_line(vec2 p, vec2 v, vec2 w) {
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
    vec2 size = dim.zw;
    vec2 pos = fragCoord.xy;
    float dist = distance(vec2(0.f, 0.f), pos);
    float angle = acos(pos.x / dist);
    if (0.f > pos.y) angle = 6.28318530718f - angle;
    float angleRange = mod(angles.y - angles.x, 6.28318530718f);
    float angleDiff = mod(angle - angles.x, 6.28318530718f);
    // How soft the edges should be (in pixels). Higher values could be used to simulate a drop shadow.
    float edgeSoftness = 1.0f;
    float theSize = size.x - 2.0 - edgeSoftness;
    // The pixel space location of the rectangle.
    vec2 location = vec2(round(size.x / 2.0), round(size.y / 2.0));
    // Calculate distance to edge.   
    float distance = dist - theSize / 2.0f;
    // Smooth the result (free antialiasing).
    float smoothedAlpha = 1.0f - smoothstep(0.0f, edgeSoftness, distance);
    // Border.  
    float borderAlpha = 1.0f - smoothstep(strokeWeight - edgeSoftness, strokeWeight, abs(distance));
    vec4 bgColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
    if (strokeWeight == 0.f) bgColor = vec4(fill.xyz, 0.0f);
    vec4 fillColor = fill;
    // When fill color alpha is 0, set to stroke color for anti-aliasing, so there's no color blending
    if (fill.w == 0.f) fillColor = vec4(stroke.xyz, 0.0f);
    fragColor = mix(bgColor, mix(fillColor, stroke, borderAlpha), smoothedAlpha);
    if (angles.x != angles.y && angleDiff > angleRange) { // Anti-alias using lines at edge of angle cutoff
        vec2 p1 = vec2(0, 0);
        vec2 p2 = vec2(cos(angles.x) * size.x, sin(angles.x) * size.x);
        vec2 p3 = vec2(cos(angles.y) * size.x, sin(angles.y) * size.x);

        float d1 = distance_point_line(pos, p1, p2);
        float d2 = distance_point_line(pos, p1, p3);
        float cutoffAlpha = smoothstep(0.f, 2.0f, min(d1, d2));
        fragColor = mix(fragColor, bgColor, cutoffAlpha);
    }
}
)