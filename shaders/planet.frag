#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 1) in vec3 inNormal;
layout(location = 3) in vec4 worldPosition;

layout(location = 0) out vec4 outColor;

void main()
{

    vec3 uColorFlat  = vec3(0.2, 0.7, 0.2); // green grass
    vec3 uColorSlope = vec3(0.6, 0.55, 0.45); // rocky
    vec3 uColorSteep = vec3(0.15, 0.15, 0.15); // dark cliff

    float uSlopeLow  = 0.25;
    float uSlopeHigh = 0.8;

    // we treat the normal as the up direction at that pixel
    vec3 N = normalize(inNormal);

    // slope relative to gravity orientation
    // if planet rotates, gravity = radial direction anyway
    float slope = 1.0 - abs(dot(vec4(inNormal, 0), normalize(worldPosition)));

    // clamp for safety
    slope = clamp(slope, -1.0, 1.0);

    // smooth blends
    float tFlat  = smoothstep(uSlopeLow, uSlopeHigh, slope);
    float tSteep = 1.0 - smoothstep(uSlopeLow, uSlopeHigh, slope);

    // 3-way blend:
    // steep <--> slope <--> flat
    vec3 col = vec3(0.0);

    // interpolate from steep to slope
    vec3 midColor = mix(uColorSteep, uColorSlope, 
                        smoothstep(uSlopeLow - 0.1, uSlopeLow + 0.1, slope));

    // interpolate from slope to flat
    col = mix(midColor, uColorFlat,
              smoothstep(uSlopeHigh - 0.1, uSlopeHigh + 0.1, slope));

    outColor = vec4(col, 1.0);
}