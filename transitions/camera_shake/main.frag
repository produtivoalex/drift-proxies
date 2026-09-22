#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float intensity;
uniform float frequency;

float hash11(float p) {
    p = fract(p * 0.1031);
    p *= p + 33.33;
    p *= p + p;
    return fract(p);
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

    // Bell-curve envelope peaking at cut point 0.5
    float env = exp(-9.0 * abs(p - 0.5)) * intensity;

    // Deterministic pseudo-random offset quantized by frequency
    float stepIndex = floor(p * frequency);
    float noiseX = (hash11(stepIndex * 13.37 + 1.2) - 0.5) * 2.0;
    float noiseY = (hash11(stepIndex * 27.81 + 7.4) - 0.5) * 2.0;
    vec2 offset = vec2(noiseX, noiseY) * 0.06 * env;

    // Slight dynamic zoom to hide edge clipping during shake
    float zoom = 1.0 + env * 0.07;
    vec2 shakenUv = (uv - 0.5 + offset) / zoom + 0.5;

    // Subtle chromatic aberration under strong shake
    float chroma = env * 0.015;
    vec4 colA;
    if (chroma > 0.0005) {
        float r = texture(u_fromTexture, shakenUv + vec2(chroma, 0.0)).r;
        float g = texture(u_fromTexture, shakenUv).g;
        float b = texture(u_fromTexture, shakenUv - vec2(chroma, 0.0)).b;
        float a = texture(u_fromTexture, shakenUv).a;
        colA = vec4(r, g, b, a);
    } else {
        colA = texture(u_fromTexture, shakenUv);
    }

    vec4 colB;
    if (chroma > 0.0005) {
        float r = texture(u_toTexture, shakenUv + vec2(chroma, 0.0)).r;
        float g = texture(u_toTexture, shakenUv).g;
        float b = texture(u_toTexture, shakenUv - vec2(chroma, 0.0)).b;
        float a = texture(u_toTexture, shakenUv).a;
        colB = vec4(r, g, b, a);
    } else {
        colB = texture(u_toTexture, shakenUv);
    }

    float blend = smoothstep(0.42, 0.58, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
