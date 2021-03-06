#version 430 core
layout(location = 0) in vec3 aPos;

uniform mat4 view;

out mat4 viewMatrix;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	viewMatrix = view;
}