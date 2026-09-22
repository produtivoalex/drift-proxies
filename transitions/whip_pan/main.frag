#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float direction;
uniform float motionBlur;

vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);
    float dir = direction >= 0.0 ? 1.0 : -1.0;

    // Fast S-curve for high velocity at midpoint
    float ease = smoothstep(0.0, 1.0, p);
    ease = mix(ease, p * p * (3.0 - 2.0 * p), 0.5);

    // Coordinate offset for outgoing and incoming
    vec2 shiftFrom = vec2(-ease * dir, 0.0);
    vec2 shiftTo = vec2((1.0 - ease) * dir, 0.0);

    // Motion blur width peaks at 0.5
    float blurWidth = sin(p * 3.14159265) * 0.04 * motionBlur;

    // Sample outgoing texture with horizontal blur
    vec4 colA = vec4(0.0);
    const int SAMPLES = 7;
    for (int i = 0; i < SAMPLES; ++i) {
        float offset = (float(i) / float(SAMPLES - 1) - 0.5) * blurWidth;
        colA += texture(u_fromTexture, uv + shiftFrom + vec2(offset, 0.0));
    }
    colA /= float(SAMPLES);

    // Sample incoming texture with horizontal blur
    vec4 colB = vec4(0.0);
    for (int i = 0; i < SAMPLES; ++i) {
        float offset = (float(i) / float(SAMPLES - 1) - 0.5) * blurWidth;
        colB += texture(u_toTexture, uv + shiftTo + vec2(offset, 0.0));
    }
    colB /= float(SAMPLES);

    float blend = smoothstep(0.40, 0.60, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
