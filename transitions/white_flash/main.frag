#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float intensity;
uniform float bloom;

vec4 over(vec4 top, vec4 bot) {
    float oa = top.a + bot.a * (1.0 - top.a);
    if (oa <= 0.0001) return vec4(0.0);
    vec3 rgb = (top.rgb * top.a + bot.rgb * bot.a * (1.0 - top.a)) / oa;
    return vec4(rgb, oa);
}

void main() {
    vec2 uv = v_texCoord;
    float p = clamp(u_progress, 0.0, 1.0);

    // Explosive flash spike at cut point 0.5
    float flash = pow(max(0.0, 1.0 - 2.0 * abs(p - 0.5)), 2.8) * intensity;

    // Outgoing and incoming textures
    vec4 colA = texture(u_fromTexture, uv);
    vec4 colB = texture(u_toTexture, uv);

    float blend = smoothstep(0.40, 0.60, p);
    colA.a *= (1.0 - blend);
    colB.a *= blend;
    vec4 base = over(colB, colA);

    // Supercharge whites and highlights
    vec3 exposed = base.rgb + vec3(flash * 1.4);
    // Vignetted glow falloff from center
    float dist = length(uv - 0.5);
    exposed += vec3(flash * bloom * (1.0 - dist * 0.7));

    fragColor = vec4(clamp(exposed, 0.0, 1.0), base.a);
}
