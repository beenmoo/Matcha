#version 460 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform vec4 u_AlbedoColor;
uniform sampler2D u_AlbedoMap;
uniform bool u_HasAlbedoMap;

void main()
{
    vec4 albedo = u_HasAlbedoMap ? texture(u_AlbedoMap, v_TexCoord) : vec4(1.0);

    o_Color = albedo * u_AlbedoColor;
}
