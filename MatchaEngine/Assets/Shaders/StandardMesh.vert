#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal; // Optional, but standard
layout (location = 2) in vec2 a_TexCoord;

// --- UNIFORM BUFFER FOR CAMERA MATRICES ---
layout (std140) uniform CameraBlock 
{
    mat4 u_ViewProjection;
};

// --- PER-OBJECT UNIFORM ---
// Individual model matrices change per object, so they stay as standard uniforms
uniform mat4 u_WorldMatrix;

out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    
    // Transform: Projection * View * World * Position
    gl_Position = u_ViewProjection * u_WorldMatrix * vec4(a_Position, 1.0f);
}
