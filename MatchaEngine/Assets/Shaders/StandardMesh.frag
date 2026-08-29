#version 460 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;
in vec3 v_Normal;
in vec3 v_WorldPosition;

// Same binding/layout as StandardMesh.vert's CameraBlock - std140 uniform blocks are matched by
// binding point, not name, but the member layout must agree between stages.
layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

uniform vec4 u_AlbedoColor;
uniform sampler2D u_AlbedoMap;
uniform float u_SpecularStrength;
uniform float u_Shininess;

// direction is the direction the light *travels* (matches how you'd naturally describe it, e.g.
// "pointing down and to the side"), not the direction toward the light - it gets flipped before
// use in calcDirLight/calcSpotLight. innerCutOff/outerCutOff are cosines of the spot cone's
// inner/outer half-angles (LightSystem converts LightComponent's degree fields before upload), so
// they can be compared directly against a dot product instead of doing an acos() per pixel.
struct Light
{
    vec3 position;
    float type;

    vec3 color;
    float intensity;

    vec3 direction;
    float range;

    float innerCutOff;
    float outerCutOff;
    vec2 padding;
};

const int MAX_LIGHTS = 16;

layout (std140, binding = 1) uniform LightBlock
{
    int u_NumLights;
    vec3 padding;

    vec3  u_AmbientColor;    // Row 1 (Global scene property)
    float u_AmbientStrength; // Row 1 (Global scene property)

    Light u_Lights[MAX_LIGHTS];
};

vec3 calcDirLight(Light light, vec3 normal, vec3 viewDir);
vec3 calcPointLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos);
vec3 calcSpotLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos);

vec3 calcDirLight(Light light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);

    // diffuse
    float diff = max(dot(normal, lightDir), 0.0f);

    // specular (Blinn-Phong): compare the normal to the halfway vector between lightDir and
    // viewDir, rather than reflecting lightDir about the normal and comparing to viewDir (Phong) -
    // cheaper, and doesn't need a hard cutoff for when the reflection points away from the viewer.
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Shininess);

    vec3 radiance = light.color * light.intensity;
    vec3 diffuse = diff * radiance;
    vec3 specular = u_SpecularStrength * spec * radiance;

    return diffuse + specular;
}

vec3 calcPointLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 lightToPixel = light.position - fragPos;
    float distance = length(lightToPixel);
    vec3 lightDir = normalize(lightToPixel);

    // Distance Cull
    if (distance > light.range) return vec3(0.0f);

    // Attenuation (Quadratic falloff)
    float attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * (distance * distance));

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0f);

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Shininess);

    vec3 radiance = light.color * light.intensity * attenuation;
    vec3 diffuse = diff * radiance;
    vec3 specular = u_SpecularStrength * spec * radiance;

    return diffuse + specular;
}

vec3 calcSpotLight(Light light, vec3 normal, vec3 viewDir, vec3 fragPos)
{
    vec3 lightToPixel = light.position - fragPos;
    float distance = length(lightToPixel);
    vec3 lightDir = normalize(lightToPixel);

    // Distance Cull
    if (distance > light.range) 
        return vec3(0.0f);

    // Attenuation
    float attenuation = 1.0f / (1.0f + 0.09f * distance + 0.032f * (distance * distance));

    // Spotlight Cone Intensity (Inner & Outer Cutoff)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = light.innerCutOff - light.outerCutOff;
    float spotIntensity = clamp((theta - light.outerCutOff) / epsilon, 0.0f, 1.0f);

    // Diffuse
    float diff = max(dot(normal, lightDir), 0.0f);

    // Specular (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), u_Shininess);

    vec3 radiance = light.color * light.intensity * attenuation * spotIntensity;
    vec3 diffuse = diff * radiance;
    vec3 specular = u_SpecularStrength * spec * radiance;

    return diffuse + specular;
}

void main()
{
    // Normalize surface normals and compute view direction
    vec3 norm = normalize(v_Normal);
    vec3 viewDir = normalize(u_CameraPosition - v_WorldPosition);

    // Compute Surface Albedo (Texture multiplied by color tint). u_AlbedoMap is always bound to a
    // real texture - a single opaque white pixel when the material has no albedo map - so this can
    // sample unconditionally instead of branching on a u_HasAlbedoMap uniform.
    vec4 albedo = texture(u_AlbedoMap, v_TexCoord) * u_AlbedoColor;

    // Global Ambient Lighting
    vec3 ambient = u_AmbientStrength * u_AmbientColor;

    vec3 totalLighting = vec3(0.0f);

    // Loop through all active lights in the scene
    for (int i = 0; i < u_NumLights; i++)
    {
        if (i >= MAX_LIGHTS) 
            break;

        float type = u_Lights[i].type;
        if (type < 0.5f) 
        {
            totalLighting += calcDirLight(u_Lights[i], norm, viewDir);
        } else if (type < 1.5f)
        {
            totalLighting += calcPointLight(u_Lights[i], norm, viewDir, v_WorldPosition);
        } else
        {
            totalLighting += calcSpotLight(u_Lights[i], norm, viewDir, v_WorldPosition);
        }
    }

    // Combine lighting with surface albedo
    vec3 result = (ambient + totalLighting) * albedo.rgb;

    // HDR Tone Mapping (Reinhard) to prevent color clipping
    result = result / (result + vec3(1.0f));

    o_Color = vec4(result, albedo.a);
}
