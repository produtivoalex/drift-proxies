#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float directionMode;
uniform float motionBlur;
uniform float chromatic;

// Straight-alpha source-over helper
vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

// Pseudo-random noise for jittered dither
float hash21(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

// Sample texture with directional chromatic aberration
vec4 sampleChromaticDir(sampler2D tex, vec2 uv, vec2 dirOffset, vec2 chromOffset) {
    vec2 uvG = uv + dirOffset;
    vec2 uvR = uvG + chromOffset;
    vec2 uvB = uvG - chromOffset;

    float r = (uvR.x >= 0.0 && uvR.x <= 1.0 && uvR.y >= 0.0 && uvR.y <= 1.0) ? texture(tex, uvR).r : 0.0;
    vec4 gSample = (uvG.x >= 0.0 && uvG.x <= 1.0 && uvG.y >= 0.0 && uvG.y <= 1.0) ? texture(tex, uvG) : vec4(0.0);
    float b = (uvB.x >= 0.0 && uvB.x <= 1.0 && uvB.y >= 0.0 && uvB.y <= 1.0) ? texture(tex, uvB).b : 0.0;

    return vec4(r, gSample.g, b, gSample.a);
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);

    // Direction vector selection:
    // 0 = Esquerda (-X), 1 = Direita (+X), 2 = Cima (+Y), 3 = Baixo (-Y)
    vec2 dirVec = vec2(-1.0, 0.0);
    int mode = int(directionMode + 0.5);
    if (mode == 1) {
        dirVec = vec2(1.0, 0.0);
    } else if (mode == 2) {
        dirVec = vec2(0.0, 1.0);
    } else if (mode == 3) {
        dirVec = vec2(0.0, -1.0);
    }

    // High velocity cubic easing around midpoint
    float ease = p < 0.5 ? 4.0 * p * p * p : 1.0 - pow(-2.0 * p + 2.0, 3.0) / 2.0;

    // Displacement offsets for outgoing (A) and incoming (B)
    vec2 shiftFrom = dirVec * ease;
    vec2 shiftTo = dirVec * (ease - 1.0);

    // Motion blur width peaks at midpoint (sine bell curve)
    float bell = sin(p * 3.14159265);
    float blurLength = bell * 0.09 * motionBlur;
    vec2 chromOffset = dirVec * bell * 0.02 * chromatic;
    float dither = (hash21(gl_FragCoord.xy) - 0.5);

    const int SAMPLES = 9;
    vec4 colA = vec4(0.0);
    vec4 colB = vec4(0.0);

    for (int i = 0; i < SAMPLES; ++i) {
        float t = (float(i) + dither) / float(SAMPLES - 1) - 0.5;
        vec2 blurOffset = dirVec * (t * blurLength);

        colA += sampleChromaticDir(u_fromTexture, uv + shiftFrom, blurOffset, chromOffset);
        colB += sampleChromaticDir(u_toTexture, uv + shiftTo, blurOffset, chromOffset);
    }

    colA /= float(SAMPLES);
    colB /= float(SAMPLES);

    // Smooth alpha blend around the cut transition
    float blend = smoothstep(0.42, 0.58, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
