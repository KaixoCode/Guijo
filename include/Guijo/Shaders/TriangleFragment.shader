LOAD_AS_STRING(
out vec4 fragColor;

uniform vec4 fill;
uniform vec4 stroke;
uniform float strokeWeight;
uniform vec2 a;
uniform vec2 b;
uniform vec2 c;

in vec2 fragCoord;

float triangle(in vec2 p, in vec2 a, in vec2 b, in vec2 c) {
    vec2 ba = b - a, cb = c - b, ac = a - c;
    vec2 pa = p - a, pb = p - b, pc = p - c;

    // Barycentric triangle areas
    float abc = ba.x * cb.y - ba.y * cb.x;
    float abp = ba.x * pa.y - ba.y * pa.x;
    float bcp = cb.x * pb.y - cb.y * pb.x;
    float cap = ac.x * pc.y - ac.y * pc.x;

    // Edge distances
    vec2 ae = pa - ba * clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    vec2 be = pb - cb * clamp(dot(pb, cb) / dot(cb, cb), 0.0, 1.0);
    vec2 ce = pc - ac * clamp(dot(pc, ac) / dot(ac, ac), 0.0, 1.0);

    // Combined edge distances
    float tri = sqrt(min(dot(ae, ae), min(dot(be, be), dot(ce, ce))));

    // Combine with the appropriate sign (-1 if inside +1 if outside)
    return tri * sign(max(-abp, max(-bcp, -cap)) * max(abp, max(bcp, cap)));
}

void main() {
    // How soft the edges should be (in pixels). Higher values could be used to simulate a drop shadow.
    float edgeSoftness = 1.0f;
    // Calculate distance to edge.   
    float distance = triangle(fragCoord.xy, a, b, c);
    // Smooth the result (free antialiasing).
    float smoothedAlpha = 1.0f - smoothstep(0.0f, edgeSoftness, distance);
    // Border.  
    float borderAlpha = 1.0f - smoothstep(strokeWeight - edgeSoftness, strokeWeight, abs(distance));
    vec4 bgColor = vec4(stroke.xyz, 0.0f);
    if (strokeWeight == 0) bgColor = vec4(fill.xyz, 0.0f);
    vec4 fillColor = fill;
    // When fill color alpha is 0, set to stroke color for anti-aliasing, so there's no color blending
    if (fill.w == 0) fillColor = vec4(stroke.xyz, 0.0f);
    fragColor = mix(bgColor, mix(fillColor, stroke, borderAlpha), smoothedAlpha);
}
)