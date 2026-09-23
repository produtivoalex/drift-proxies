#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float burnIntensity;
uniform float rollSpeed;
uniform float lightLeakWarmth;
uniform float grainAmount;

// Straight-alpha source-over helper
vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

// Pseudo-random noise functions
float hash21(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// 2D Value Noise
float valueNoise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);
    float a = hash21(i);
    float b = hash21(i + vec2(1.0, 0.0));
    float c = hash21(i + vec2(0.0, 1.0));
    float d = hash21(i + vec2(1.0, 1.0));
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion for organic paper/film burn
float fbm(vec2 p) {
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        v += amp * valueNoise(p);
        p *= 2.08;
        amp *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);

    // Peak burn and leak envelope
    float bell = sin(p * 3.14159265);
    float ease = p < 0.5 ? 4.0 * p * p * p : 1.0 - pow(-2.0 * p + 2.0, 3.0) / 2.0;

    // Gate weave (subtle analog projector jitter)
    float jitterY = (hash21(vec2(floor(p * 24.0), 1.0)) - 0.5) * 0.008 * bell;
    float jitterX = (hash21(vec2(floor(p * 24.0), 2.0)) - 0.5) * 0.004 * bell;

    // Film vertical roll
    float rollY = ease * rollSpeed;
    float rawY_A = uv.y + rollY + jitterY;
    float rawY_B = uv.y + rollY - rollSpeed + jitterY;

    vec2 uvA = vec2(uv.x + jitterX, fract(rawY_A));
    vec2 uvB = vec2(uv.x + jitterX, fract(rawY_B));

    // Sample incoming and outgoing
    vec4 colA = texture(u_fromTexture, uvA);
    vec4 colB = texture(u_toTexture, uvB);

    // 35mm Film Frame Line (the black divider bar between film frames)
    float frameBorderDistA = min(fract(rawY_A), 1.0 - fract(rawY_A));
    float frameLineA = smoothstep(0.0, 0.025, frameBorderDistA);
    colA.rgb *= frameLineA;

    float frameBorderDistB = min(fract(rawY_B), 1.0 - fract(rawY_B));
    float frameLineB = smoothstep(0.0, 0.025, frameBorderDistB);
    colB.rgb *= frameLineB;

    // Organic film burn erosion map
    float noiseVal = fbm(uv * 3.2 + vec2(p * 1.5, p * 0.8));
    float burnThreshold = mix(-0.2, 1.2, p);
    float burnMask = smoothstep(burnThreshold - 0.18 * burnIntensity, burnThreshold + 0.18 * burnIntensity, noiseVal);

    // Blend between clip A and B driven by the burn edge
    vec4 mixedCol = mix(colB, colA, burnMask);

    // Fiery burn edge halation (glow border where film melts)
    float edgeDist = abs(noiseVal - burnThreshold);
    float glowBorder = smoothstep(0.12 * burnIntensity, 0.0, edgeDist) * bell;

    // Warm light leak bloom (orange / amber / hot white)
    vec3 leakColorWarm = vec3(1.0, 0.45, 0.1) * lightLeakWarmth;
    vec3 leakColorHot = vec3(1.0, 0.95, 0.7);
    vec3 leakGlow = mix(leakColorWarm, leakColorHot, glowBorder * 0.8) * glowBorder * 2.5;

    // Corner / edge light leak wash
    float cornerLeak = pow(max(0.0, 1.0 - length(uv - vec2(0.9, 0.1))), 2.0) * bell * lightLeakWarmth * 0.9;
    vec3 cornerGlow = vec3(1.0, 0.55, 0.15) * cornerLeak;

    // Apply burns and glows additively
    mixedCol.rgb += (leakGlow + cornerGlow) * mixedCol.a;

    // Film grain overlay (Vision3 analog noise)
    if (grainAmount > 0.01) {
        float grain = (hash21(uv * u_resolution + vec2(p * 99.0, p * 33.0)) - 0.5) * grainAmount * (0.15 + bell * 0.25);
        mixedCol.rgb += vec3(grain * mixedCol.a);
    }

    fragColor = mixedCol;
}
