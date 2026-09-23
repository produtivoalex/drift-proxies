#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_fromTexture;
uniform sampler2D u_toTexture;
uniform vec2 u_resolution;
uniform float u_progress;

uniform float tearDirection;
uniform float fiberWidth;
uniform float roughness;
uniform float shadowDepth;
uniform float paperAngle;

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

// 1D/2D Value Noise
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

// Fractal Brownian Motion for jagged torn paper edge
float fbm(vec2 p) {
    float v = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; ++i) {
        v += amp * valueNoise(p);
        p *= 2.15;
        amp *= 0.5;
    }
    return v;
}

void main() {
    vec2 uv = v_texCoord;
    vec2 center = vec2(0.5, 0.5);
    float aspect = u_resolution.x / max(u_resolution.y, 1.0);
    float p = clamp(u_progress, 0.0, 1.0);

    // Stop-motion step quantization (18 fps tactile feel)
    float timeStep = floor(p * 18.0) / 18.0;
    float smoothP = mix(p, timeStep, 0.35);

    // Direction vector of the tear
    float baseAngle = 0.0;
    int dirMode = int(tearDirection + 0.5);
    if (dirMode == 1) {
        baseAngle = radians(paperAngle); // Diagonal
    } else if (dirMode == 2) {
        baseAngle = radians(90.0);       // Vertical
    } else {
        baseAngle = radians(0.0);        // Horizontal
    }

    vec2 tearAxis = vec2(cos(baseAngle), sin(baseAngle));
    vec2 transverseAxis = vec2(-sin(baseAngle), cos(baseAngle));

    // Project coordinates
    vec2 aspectUv = (uv - center);
    aspectUv.x *= aspect;

    float tearCoord = dot(aspectUv, tearAxis);
    float transCoord = dot(aspectUv, transverseAxis);

    // Multi-scale procedural paper tear edge
    float macroRip = (fbm(vec2(transCoord * 4.5, 1.3)) - 0.5) * 0.16 * roughness;
    float microFiber = (valueNoise(vec2(transCoord * 28.0, 5.7)) - 0.5) * 0.03 * roughness;
    float paperTexture = (hash21(uv * u_resolution * 0.5) - 0.5) * 0.08;

    // Tear progression sweep
    float ripExtent = 0.85 * max(aspect, 1.0);
    float ripPosition = mix(-ripExtent, ripExtent, smoothP);
    float ripDist = tearCoord - (ripPosition + macroRip + microFiber);

    // Samples
    vec4 colA = texture(u_fromTexture, uv);
    vec4 colB = texture(u_toTexture, uv);

    // Paper fiber border width
    float fWidth = max(0.005, fiberWidth);

    vec4 finalColor;

    if (ripDist < 0.0) {
        // Exposed bottom layer (Clip B)
        // Soft drop shadow cast by the torn top layer
        float shadowLength = 0.08 * shadowDepth;
        float shadow = smoothstep(-shadowLength, 0.0, ripDist);
        colB.rgb *= mix(1.0, 0.38, shadow);
        finalColor = colB;
    } else if (ripDist < fWidth) {
        // White torn paper cellulose fiber border
        float fiberFade = smoothstep(0.0, fWidth * 0.3, ripDist) * smoothstep(fWidth, fWidth * 0.7, ripDist);
        vec3 paperWhite = vec3(0.96, 0.95, 0.91) + vec3(paperTexture);
        
        // Blend edge of fiber into clip A
        finalColor = vec4(mix(colA.rgb, paperWhite, 0.92), colA.a);
    } else {
        // Top layer (Clip A)
        finalColor = colA;
    }

    fragColor = finalColor;
}
