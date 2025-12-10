#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec4 worldPosition;
layout(location = 2) in vec4 planetPosition;
layout(location = 3) in vec4 lightPos;
layout(location = 4) in vec4 lightColor;
layout(location = 5) in float planetRadius;

layout(location = 0) out vec4 outColor;

void main()
{

    // terrain colors
    vec3 uColorFlat  = vec3(0.2, 0.7, 0.2); // green grass
    vec3 uColorSlope = vec3(0.6, 0.55, 0.45); // rocky
    vec3 uColorSteep = vec3(0.15, 0.15, 0.15); // dark cliff
    vec3 uColorSnow  = vec3(1.0); // pure white

    // slope divisions
    float uSlopeLow   = 0.25;
    float uSlopeHigh  = 0.6;

    // elevation divisions
    float uSnowStart  = 0.6; // snow fade
    float uSnowEnd    = 0.8; // full snow

    // Normal of the terrain (mesh face)
    vec3 N = normalize(inNormal);
    // Direction from planet center
    vec3 terrainL = normalize(worldPosition.xyz - planetPosition.xyz);

    // normalized elevation (based on planet radius)
    float elevation = length(worldPosition.xyz - planetPosition.xyz);
    elevation = (elevation - planetRadius) / planetRadius;

    // vec3 col;
    // if (planetPosition.w == 0) {
    //     col = vec3(0.0);
    // } else {
    //     col = vec3(1.0);
    // }
    //vec3 col = mix(vec3(0.0), vec3(1.0), elevation);

    // Calculate slope from face and center normals
    float slope = 1.0 - abs(dot(N, terrainL));
    slope = clamp(slope, 0.0, 1.0);

    // fade between slope and steep
    vec3 midColor = mix(
        uColorSlope,
        uColorSteep,
        smoothstep(uSlopeHigh - 0.1, uSlopeHigh + 0.1, slope)
    );

    //fade between flat and slope
    vec3 col = mix(
        uColorFlat,
        midColor,
        smoothstep(uSlopeLow - 0.1, uSlopeLow + 0.1, slope)
    );

    // // // --- elevation blending ---
    // float tSnow = smoothstep(uSnowStart, uSnowEnd, elevation);

    // // // final result: lerp base terrain to snow
    // col = mix(col, uColorSnow, tSnow);

    // Calculate sun lighthing
    // direction of sunlight
    vec3 sunL = normalize(worldPosition.xyz - lightPos.xyz);

    // Lambertian diffuse
    float diffuse = max(dot(N, -sunL), 0.0);

    // prevents 100% dark
    float ambient = 0.3;
    vec3 sunColor = vec3(1.0, 0.98, 0.9);
    // apply sunlight to planet color
    col.rgb *= sunColor * (ambient + diffuse * (1.0 - ambient));

    outColor = vec4(col, 1.0);
}