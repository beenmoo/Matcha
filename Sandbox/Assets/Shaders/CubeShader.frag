#version 460 core

in vec3 v_Color;
out vec4 o_FragColor;

void main()
{
	o_FragColor = vec4(v_Color, 1.0f);
}