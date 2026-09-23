#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float zoomIntensity;
uniform float motionBlur;
uniform float rotation;
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

// Transform UV with aspect ratio, rotation angle in radians, and scale
vec2 transformUv(vec2 uv, vec2 center, float scale, float rotAngle, float aspect) {
    vec2 delta = uv - center;
    delta.x *= aspect;
    float c = cos(rotAngle);
    float s = sin(rotAngle);
    mat2 rotMat = mat2(c, -s, s, c);
    delta = rotMat * delta;
    delta.x /= aspect;
    return delta / max(scale, 0.0001) + center;
}

// Sample texture with chromatic aberration and alpha bounds check
vec4 sampleChromatic(sampler2D tex, vec2 uv, vec2 center, float chromaticOffset) {
    vec2 dir = uv - center;
    vec2 uvR = uv - dir * chromaticOffset;
    vec2 uvG = uv;
    vec2 uvB = uv + dir * chromaticOffset;

    float r = (uvR.x >= 0.0 && uvR.x <= 1.0 && uvR.y >= 0.0 && uvR.y <= 1.0) ? texture(tex, uvR).r : 0.0;
    vec4 gSample = (uvG.x >= 0.0 && uvG.x <= 1.0 && uvG.y >= 0.0 && uvG.y <= 1.0) ? texture(tex, uvG) : vec4(0.0);
    float b = (uvB.x >= 0.0 && uvB.x <= 1.0 && uvB.y >= 0.0 && uvB.y <= 1.0) ? texture(tex, uvB).b : 0.0;

    return vec4(r, gSample.g, b, gSample.a);
}

void main() {
    vec2 uv = v_texCoord;
    vec2 center = vec2(0.5, 0.5);
    float aspect = u_resolution.x / max(u_resolution.y, 1.0);
    float p = clamp(u_progress, 0.0, 1.0);

    // Dynamic curve peaking at p = 0.5
    float velocityBell = sin(p * 3.14159265);
    float rotAngle = radians(rotation) * velocityBell;

    // Scale curves for Zoom Out:
    // Outgoing clip A shrinks into the distance from 1.0 down to (1.0 / zoomIntensity)
    float tA = clamp(p * 2.0, 0.0, 1.0);
    float scaleA = mix(1.0, 1.0 / zoomIntensity, tA * tA * (3.0 - 2.0 * tA));

    // Incoming clip B starts massively zoomed in (zoomIntensity) and snaps down to 1.0
    float tB = clamp((p - 0.45) / 0.55, 0.0, 1.0);
    float scaleB = mix(zoomIntensity, 1.0, sqrt(tB));

    // Base transformed coordinates
    vec2 uvA = transformUv(uv, center, scaleA, rotAngle, aspect);
    vec2 uvB = transformUv(uv, center, scaleB, rotAngle * (1.0 - tB), aspect);

    // Radial motion blur parameters (converging inward)
    float blurAmount = velocityBell * 0.05 * motionBlur;
    float chromOffset = velocityBell * 0.015 * chromatic;
    float dither = (hash21(gl_FragCoord.xy) - 0.5);

    const int SAMPLES = 9;
    vec4 colA = vec4(0.0);
    vec4 colB = vec4(0.0);

    for (int i = 0; i < SAMPLES; ++i) {
        float stepNorm = (float(i) + dither) / float(SAMPLES - 1) - 0.5;
        float radialFactor = stepNorm * blurAmount;

        vec2 sampleUvA = uvA + (uvA - center) * radialFactor;
        vec2 sampleUvB = uvB + (uvB - center) * radialFactor;

        colA += sampleChromatic(u_fromTexture, sampleUvA, center, chromOffset);
        colB += sampleChromatic(u_toTexture, sampleUvB, center, chromOffset);
    }

    colA /= float(SAMPLES);
    colB /= float(SAMPLES);

    // Crossfade curve around midpoint
    float blend = smoothstep(0.42, 0.58, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
