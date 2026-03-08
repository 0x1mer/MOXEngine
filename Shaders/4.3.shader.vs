#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
layout (location = 3) in vec3 aColor;

out vec3 vColor;
out vec3 vNormal;   // world-space
out vec3 vPosWS;    // world-space
out vec2 vTex;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 posWS = model * vec4(aPos, 1.0);
    vPosWS = posWS.xyz;

    mat3 normalMat = mat3(transpose(inverse(model)));

    vNormal = normalize(normalMat * aNormal);
    vTex    = aTexCoord;
    vColor = aColor;

    gl_Position = projection * view * posWS;
}
