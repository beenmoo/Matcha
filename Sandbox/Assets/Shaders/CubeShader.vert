#version 460 core

layout (location = 0) in vec3 a_Pos;
layout (location = 1) in vec3 a_Color;

uniform mat4 u_MVP;

out vec3 v_Color;

void main()
{
	gl_Position = u_MVP * vec4(a_Pos, 1.0f);
	v_Color = a_Color;
}