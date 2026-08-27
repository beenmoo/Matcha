#version 460 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

uniform vec4 u_AlbedoColor;

void main()
{
    // Just output a solid color or texture coordinate tint
    o_Color = u_AlbedoColor;
}
