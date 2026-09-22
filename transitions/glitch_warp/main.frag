#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float glitchAmount;
uniform float sliceBands;

float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
}

float hash21(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);

    // Peak glitch displacement in the middle
    float env = sin(p * 3.14159265) * glitchAmount;

    // Quantized time step for snappy digital glitches
    float stepIndex = floor(p * 18.0);
    float bandIndex = floor(uv.y * sliceBands);

    // Pseudo-random horizontal displacement per band
    float shift = (hash21(vec2(bandIndex, stepIndex)) - 0.5) * 0.14 * env;

    // Outgoing with RGB channel split
    float rA = texture(u_fromTexture, uv + vec2(shift, 0.0)).r;
    float gA = texture(u_fromTexture, uv + vec2(shift * 0.4, 0.0)).g;
    float bA = texture(u_fromTexture, uv - vec2(shift * 0.7, 0.0)).b;
    float aA = texture(u_fromTexture, uv).a;
    vec4 colA = vec4(rA, gA, bA, aA);

    // Incoming with RGB channel split
    float rB = texture(u_toTexture, uv + vec2(shift * 0.8, 0.0)).r;
    float gB = texture(u_toTexture, uv - vec2(shift * 0.3, 0.0)).g;
    float bB = texture(u_toTexture, uv - vec2(shift * 0.9, 0.0)).b;
    float aB = texture(u_toTexture, uv).a;
    vec4 colB = vec4(rB, gB, bB, aB);

    float blend = smoothstep(0.40, 0.60, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
