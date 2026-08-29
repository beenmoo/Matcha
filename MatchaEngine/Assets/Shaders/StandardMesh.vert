#version 460 core

layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec2 a_TexCoord;

// --- UNIFORM BUFFER FOR CAMERA MATRICES ---
layout (std140, binding = 0) uniform CameraBlock
{
    mat4 u_ViewProjection;
    vec3 u_CameraPosition;
};

// --- PER-OBJECT UNIFORM ---
// Individual model matrices change per object, so they stay as standard uniforms
uniform mat4 u_WorldMatrix;

out vec2 v_TexCoord;
out vec3 v_Normal;
out vec3 v_WorldPosition;

void main()
{
    v_TexCoord = a_TexCoord;

    // transpose(inverse(...)) rather than u_WorldMatrix directly: a plain model-matrix multiply
    // only preserves normal direction under uniform scale. Under non-uniform scale it stops being
    // perpendicular to the (also transformed) surface - this "normal matrix" is what keeps it
    // correct in general. Upper-left 3x3 only: normals are directions, so translation doesn't
    // apply to them.
    v_Normal = mat3(transpose(inverse(u_WorldMatrix))) * a_Normal;
    v_WorldPosition = vec3(u_WorldMatrix * vec4(a_Position, 1.0f));

    // Transform: Projection * View * World * Position
    gl_Position = u_ViewProjection * u_WorldMatrix * vec4(a_Position, 1.0f);
}
