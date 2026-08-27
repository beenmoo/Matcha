#version 460 core

layout (location = 0) out vec4 o_Color;

in vec2 v_TexCoord;

// --- MATERIAL UNIFORMS ---
uniform vec4 u_AlbedoColor;
uniform sampler2D u_Texture;
uniform bool u_HasTexture; // Switch to toggle between solid color and textured rendering

void main()
{
    if (u_HasTexture)
    {
        // Multiply texture color by the tint color
        o_Color = texture(u_Texture, v_TexCoord) * u_AlbedoColor;
    }
    else
    {
        // Just output the solid color tint
        o_Color = u_AlbedoColor;
    }
}