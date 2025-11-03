#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec4 colorMod;
layout(location = 3) in vec4 worldPosition;
layout(location = 4) in vec4 cameraPos;
layout(location = 5) in vec4 lightPos;
layout(location = 6) in vec4 lightColor;

layout(location = 0) out vec4 outColor;


void main()
{
    vec3 N = normalize(inNormal);          // fragment normal
    vec3 L = normalize(worldPosition.xyz - lightPos.xyz); // from origin to fragment

    float diffuse = max(dot(N, -L), 0.0);   // Lambertian diffuse
    float ambient = 0.1;                   // minimum brightness

    vec3 sunColor = vec3(1.0, 0.98, 0.9); // optional tint

    vec4 texColor = texture(texSampler, fragTexCoord);
    texColor.rgb *= sunColor * (ambient + diffuse * (1.0 - ambient));

    outColor = texColor * colorMod;
}
