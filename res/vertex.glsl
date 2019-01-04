#version 430 core

layout (location = 0) in vec2 vertexPosition;

uniform mat4 mvp;

out vec4 color;

void main()
{
	gl_Position = mvp * vec4(vertexPosition, 0, 1);
}
