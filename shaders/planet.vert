#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4    model;
    mat4    view;
    mat4    proj;
    vec4    planetPosition;
    vec4    lightPos;
    vec4    lightColor;
} ubo;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec4 outWorldPosition;
layout(location = 2) out vec4 outPlanetPosition;
layout(location = 3) out vec4 outLightPos;
layout(location = 4) out vec4 outLightColor;
layout(location = 5) out float outPlanetRadius;

void main()
{
    mat3 normalMatrix;
    mat4 mvp = ubo.proj * ubo.view * ubo.model;

    //positions
    gl_Position =  mvp * vec4(inPosition, 1.0);
    outWorldPosition = ubo.model * vec4(inPosition,1.0);

    //normals
    normalMatrix = transpose(inverse(mat3(ubo.model)));
    outNormal = normalize(normalMatrix*inNormal);

    //passthroughs
    outPlanetPosition = ubo.planetPosition;
    outPlanetRadius = ubo.planetPosition.w;
    outLightPos = ubo.lightPos;
    outLightColor = ubo.lightColor;
}
