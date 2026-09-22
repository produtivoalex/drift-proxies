#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float punchScale;
uniform float chromatic;

vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

void main() {
    vec2 uv = v_texCoord;
    vec2 center = vec2(0.5, 0.5);

    float p = clamp(u_progress, 0.0, 1.0);
    float mid = 0.5;

    // Fast exponential acceleration into 0.5, then elastic deceleration to 1.0
    float scaleA = mix(1.0, punchScale, p * 2.0 * p * 2.0);
    float tB = (p - mid) * 2.0;
    // Elastic settle: starts slightly above 1.0 and bounces down to 1.0
    float scaleB = mix(punchScale * 0.9, 1.0, clamp(tB * (1.5 - 0.5 * tB), 0.0, 1.0));

    // Chromatic offset based on zoom velocity
    float fringing = sin(p * 3.14159265) * chromatic * 0.02;

    vec2 uvA = (uv - center) / max(scaleA, 0.001) + center;
    vec2 uvB = (uv - center) / max(scaleB, 0.001) + center;

    vec4 colA;
    if (fringing > 0.0001) {
        float r = texture(u_fromTexture, uvA + (uvA - center) * fringing).r;
        float g = texture(u_fromTexture, uvA).g;
        float b = texture(u_fromTexture, uvA - (uvA - center) * fringing).b;
        float a = texture(u_fromTexture, uvA).a;
        colA = vec4(r, g, b, a);
    } else {
        colA = texture(u_fromTexture, uvA);
    }

    vec4 colB;
    if (fringing > 0.0001) {
        float r = texture(u_toTexture, uvB + (uvB - center) * fringing).r;
        float g = texture(u_toTexture, uvB).g;
        float b = texture(u_toTexture, uvB - (uvB - center) * fringing).b;
        float a = texture(u_toTexture, uvB).a;
        colB = vec4(r, g, b, a);
    } else {
        colB = texture(u_toTexture, uvB);
    }

    float blend = smoothstep(0.42, 0.58, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;

    fragColor = over(colB, colA);
}
