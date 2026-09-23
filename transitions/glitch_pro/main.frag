#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float glitchIntensity;
uniform float blockScale;
uniform float chromaticShift;
uniform float noiseJitter;

// Straight-alpha source-over helper
vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

// 1D & 2D Hash functions
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

// Sample texture with chromatic split and digital block displacement
vec4 sampleGlitch(sampler2D tex, vec2 uv, vec2 blockDisp, vec2 chromVector, float colorInvert) {
    vec2 baseUv = uv + blockDisp;
    vec2 uvR = baseUv + chromVector;
    vec2 uvG = baseUv;
    vec2 uvB = baseUv - chromVector;

    float r = (uvR.x >= 0.0 && uvR.x <= 1.0 && uvR.y >= 0.0 && uvR.y <= 1.0) ? texture(tex, uvR).r : 0.0;
    vec4 gSample = (uvG.x >= 0.0 && uvG.x <= 1.0 && uvG.y >= 0.0 && uvG.y <= 1.0) ? texture(tex, uvG) : vec4(0.0);
    float b = (uvB.x >= 0.0 && uvB.x <= 1.0 && uvB.y >= 0.0 && uvB.y <= 1.0) ? texture(tex, uvB).b : 0.0;

    vec3 rgb = vec3(r, gSample.g, b);
    if (colorInvert > 0.5) {
        rgb = vec3(1.0) - rgb;
    }
    return vec4(rgb, gSample.a);
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);

    // Glitch envelope: bell curve with sharp snappy pulses
    float bell = sin(p * 3.14159265);
    float pulse = pow(bell, 1.5);

    // Quantize time into discrete digital steps (24 fps glitch cadence)
    float timeStep = floor(p * 24.0);

    // Macro 2D block subdivision
    vec2 blockCoord = floor(uv * vec2(blockScale * 0.4, blockScale));
    float blockHash = hash21(blockCoord + vec2(timeStep * 19.31, timeStep * 7.73));
    float isBlockGlitch = step(0.65, blockHash);

    // Horizontal block tear displacement
    float blockTear = (blockHash - 0.5) * 0.22 * glitchIntensity * isBlockGlitch * pulse;

    // High frequency scanline micro-jitter
    float lineIndex = floor(uv.y * u_resolution.y * 0.35);
    float lineHash = hash11(lineIndex + timeStep * 13.1);
    float isLineGlitch = step(0.78, lineHash);
    float lineTear = (lineHash - 0.5) * 0.08 * noiseJitter * isLineGlitch * pulse;

    vec2 totalDisp = vec2(blockTear + lineTear, 0.0);

    // 2D Chromatic vector (horizontal dominant with subtle vertical split)
    vec2 chromVector = vec2(0.038 * chromaticShift * pulse, 0.007 * chromaticShift * pulse * (blockHash - 0.5));

    // Digital corruption: occasional color flash / invert on glitch blocks
    float colorInvert = (isBlockGlitch > 0.5 && blockHash > 0.93 && bell > 0.6) ? 1.0 : 0.0;

    // Sample outgoing (A) and incoming (B)
    vec4 colA = sampleGlitch(u_fromTexture, uv, totalDisp, chromVector, colorInvert);
    vec4 colB = sampleGlitch(u_toTexture, uv, totalDisp, chromVector, colorInvert);

    // Scanline static overlay on glitch regions
    if (noiseJitter > 0.01 && pulse > 0.1) {
        float scanline = sin(uv.y * u_resolution.y * 1.5) * 0.5 + 0.5;
        float noise = hash21(uv * u_resolution + vec2(timeStep, timeStep)) * 0.15 * noiseJitter * pulse;
        colA.rgb += vec3((scanline * 0.04 + noise) * colA.a);
        colB.rgb += vec3((scanline * 0.04 + noise) * colB.a);
    }

    // Cut transition blend
    float blend = smoothstep(0.42, 0.58, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
