#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float flashBrightness;
uniform float streakWidth;
uniform float flareColorWarmth;
uniform float aberration;

// Straight-alpha source-over helper
vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

// Sample texture with chromatic dispersion
vec4 sampleChromaticFlare(sampler2D tex, vec2 uv, vec2 center, float offset) {
    vec2 dir = uv - center;
    vec2 uvR = uv + dir * offset;
    vec2 uvG = uv;
    vec2 uvB = uv - dir * offset;

    float r = (uvR.x >= 0.0 && uvR.x <= 1.0 && uvR.y >= 0.0 && uvR.y <= 1.0) ? texture(tex, uvR).r : 0.0;
    vec4 gSample = (uvG.x >= 0.0 && uvG.x <= 1.0 && uvG.y >= 0.0 && uvG.y <= 1.0) ? texture(tex, uvG) : vec4(0.0);
    float b = (uvB.x >= 0.0 && uvB.x <= 1.0 && uvB.y >= 0.0 && uvB.y <= 1.0) ? texture(tex, uvB).b : 0.0;

    return vec4(r, gSample.g, b, gSample.a);
}

void main() {
    vec2 uv = v_texCoord;
    float aspect = u_resolution.x / max(u_resolution.y, 1.0);
    float p = clamp(u_progress, 0.0, 1.0);

    // Peak flare flash envelope in the middle
    float bell = sin(p * 3.14159265);
    float flashPeak = pow(bell, 2.2) * flashBrightness;

    // Flare source sweeps slightly across the center
    vec2 flareOrigin = vec2(0.5 + (p - 0.5) * 0.35, 0.5);

    // Chromatic dispersion offset
    float chromOffset = bell * 0.02 * aberration;

    // Sample outgoing and incoming frames
    vec4 colA = sampleChromaticFlare(u_fromTexture, uv, flareOrigin, chromOffset);
    vec4 colB = sampleChromaticFlare(u_toTexture, uv, flareOrigin, chromOffset);

    // Cut transition blend
    float blend = smoothstep(0.40, 0.60, p);
    vec4 mixedCol = mix(colA, colB, blend);

    // Exposure blowout (overexposure bloom)
    float blowout = pow(bell, 3.0) * flashBrightness * 1.2;
    mixedCol.rgb = mix(mixedCol.rgb, vec3(1.0), clamp(blowout, 0.0, 1.0));

    // Anamorphic horizontal streak optics
    float dy = abs(uv.y - flareOrigin.y) * 55.0 / max(streakWidth, 0.05);
    float dx = abs(uv.x - flareOrigin.x) * 2.2;
    float anamorphicLine = exp(-dy * dy) * exp(-dx * dx) * flashPeak * 2.5;

    // Radial core glow and optical ring halo
    vec2 aspectDist = (uv - flareOrigin);
    aspectDist.x *= aspect;
    float coreDist = length(aspectDist);

    float coreGlow = exp(-coreDist * 5.5) * flashPeak * 2.0;
    float ringHalo = exp(-pow((coreDist - 0.28) * 20.0, 2.0)) * flashPeak * 0.5;

    // Palette: Cyan/Blue (0.0) vs Golden Amber (1.0)
    vec3 cyanBlue = vec3(0.35, 0.72, 1.0);
    vec3 goldenAmber = vec3(1.0, 0.75, 0.32);
    vec3 flareTint = mix(cyanBlue, goldenAmber, flareColorWarmth);

    // Composite optical flares additively
    vec3 streakRgb = mix(flareTint, vec3(1.0), 0.6) * anamorphicLine;
    vec3 glowRgb = flareTint * (coreGlow + ringHalo);

    mixedCol.rgb += (streakRgb + glowRgb) * mixedCol.a;

    fragColor = mixedCol;
}
