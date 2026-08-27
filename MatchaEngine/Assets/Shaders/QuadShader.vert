#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec2 a_TexCoord;

// --- GLOBAL CAMERA UBO (Shared with 3D objects) ---
layout (std140, binding = 0) uniform CameraBlock 
{
    mat4 u_ViewProjection;
};

// --- PER-OBJECT UNIFORMS ---
uniform mat4 u_WorldMatrix;

// --- OUTPUTS TO FRAGMENT SHADER ---
out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    
    // Transform position into clip space
    gl_Position = u_ViewProjection * u_WorldMatrix * vec4(a_Position, 1.0f);
}