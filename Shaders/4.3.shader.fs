#version 430 core

in vec3 vColor;
in vec3 vNormal;
in vec3 vPosWS;
in vec2 vTex;

out vec4 FragColor;

uniform sampler2D uAtlas;
uniform vec3 lightDir;
uniform vec3 ambient;
uniform float uTime;
uniform bool isLightEnabled;

float minY = -20;
float maxY = 20;

vec3 CalculateLight(vec3 N, vec3 L, bool enabled)
{
    if (!enabled)
        return vec3(1.0);

    float diff = max(dot(N, L), 0.0);
    return ambient + diff;
}

void main()
{
    vec3 N = normalize(vNormal);
    vec3 L = normalize(-lightDir);

    vec3 texColor = texture(uAtlas, vTex).rgb;

    vec3 lit = CalculateLight(N, L, isLightEnabled);
    FragColor = vec4(texColor * lit, 1.0);
}