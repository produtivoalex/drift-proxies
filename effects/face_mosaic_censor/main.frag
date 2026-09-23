#version 330 core
in vec2 v_texCoord;
out vec4 fragColor;

uniform sampler2D u_currentTexture;
uniform vec2 u_resolution;

uniform float u_faceValid;
uniform float u_faceCenterX;
uniform float u_faceCenterY;
uniform float u_faceRx;
uniform float u_faceRy;
uniform float u_faceAngle;
uniform float u_faceLeftEyeX;
uniform float u_faceLeftEyeY;
uniform float u_faceRightEyeX;
uniform float u_faceRightEyeY;

uniform float mode; // 0 = Pixelate/Mosaico, 1 = Blur/Desfoque, 2 = Barra Preta nos Olhos (Eyes Bar)
uniform float pixelSize; // 4.0 to 64.0
uniform float radius; // 0.6 to 2.5
uniform float feather; // 0.02 to 0.5

// Convert to aspect-corrected local coordinates
vec2 toLocal(vec2 uv, float aspect) {
    return vec2(uv.x, uv.y * aspect);
}

vec2 rotate2D(vec2 p, float angle) {
    float s = sin(angle);
    float c = cos(angle);
    return vec2(p.x * c - p.y * s, p.x * s + p.y * c);
}

void main() {
    if (u_faceValid < 0.5) {
        fragColor = texture(u_currentTexture, v_texCoord);
        return;
    }

    float aspect = u_resolution.y / u_resolution.x;
    vec2 uv = v_texCoord;

    // Mode 2: Censura de Olhos (Faixa Preta clássica jornalística nos olhos)
    if (mode > 1.5) {
        vec2 eyeMid = 0.5 * vec2(u_faceLeftEyeX + u_faceRightEyeX, u_faceLeftEyeY + u_faceRightEyeY);
        vec2 eyeMidLocal = toLocal(eyeMid, aspect);
        vec2 pLocal = toLocal(uv, aspect);
        vec2 d = rotate2D(pLocal - eyeMidLocal, -u_faceAngle);

        float eyeDist = length(toLocal(vec2(u_faceRightEyeX - u_faceLeftEyeX, u_faceRightEyeY - u_faceLeftEyeY), aspect));
        float barHalfW = max(0.04, eyeDist * 0.9 * radius);
        float barHalfH = max(0.015, eyeDist * 0.35 * radius);

        if (abs(d.x) <= barHalfW && abs(d.y) <= barHalfH) {
            fragColor = vec4(0.0, 0.0, 0.0, 1.0);
            return;
        }
        fragColor = texture(u_currentTexture, uv);
        return;
    }

    // Modes 0 e 1: Mosaico ou Desfoque de Rosto Inteiro
    vec2 centerLocal = toLocal(vec2(u_faceCenterX, u_faceCenterY), aspect);
    vec2 pLocal = toLocal(uv, aspect);
    vec2 d = rotate2D(pLocal - centerLocal, -u_faceAngle);

    float rx = max(0.02, u_faceRx * radius);
    float ry = max(0.02, u_faceRy * radius * aspect);

    // Distância elíptica normalizada (0 no centro, 1 na borda da elipse facial)
    float ellipDist = length(vec2(d.x / rx, d.y / ry));

    if (ellipDist >= 1.0 + feather) {
        fragColor = texture(u_currentTexture, uv);
        return;
    }

    // Fator de mistura suave para a borda
    float alpha = 1.0 - smoothstep(1.0 - feather, 1.0 + feather, ellipDist);

    vec4 censorColor;
    if (mode < 0.5) {
        // Mosaico (Pixelate)
        float numBlocksX = max(4.0, u_resolution.x / max(4.0, pixelSize));
        float numBlocksY = max(4.0, u_resolution.y / max(4.0, pixelSize));
        vec2 pixUV = vec2((floor(uv.x * numBlocksX) + 0.5) / numBlocksX,
                          (floor(uv.y * numBlocksY) + 0.5) / numBlocksY);
        censorColor = texture(u_currentTexture, pixUV);
    } else {
        // Desfoque Gaussiano 9-tap
        vec4 acc = vec4(0.0);
        float blurRadius = max(4.0, pixelSize * 0.75);
        vec2 blurStep = vec2(blurRadius / u_resolution.x, blurRadius / u_resolution.y);
        
        acc += texture(u_currentTexture, uv + vec2(-blurStep.x, -blurStep.y)) * 0.075;
        acc += texture(u_currentTexture, uv + vec2(0.0, -blurStep.y)) * 0.125;
        acc += texture(u_currentTexture, uv + vec2(blurStep.x, -blurStep.y)) * 0.075;
        
        acc += texture(u_currentTexture, uv + vec2(-blurStep.x, 0.0)) * 0.125;
        acc += texture(u_currentTexture, uv) * 0.200;
        acc += texture(u_currentTexture, uv + vec2(blurStep.x, 0.0)) * 0.125;
        
        acc += texture(u_currentTexture, uv + vec2(-blurStep.x, blurStep.y)) * 0.075;
        acc += texture(u_currentTexture, uv + vec2(0.0, blurStep.y)) * 0.125;
        acc += texture(u_currentTexture, uv + vec2(blurStep.x, blurStep.y)) * 0.075;
        censorColor = acc;
    }

    vec4 origColor = texture(u_currentTexture, uv);
    fragColor = mix(origColor, censorColor, alpha);
}
