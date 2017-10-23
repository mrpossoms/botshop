#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 view_projection;
uniform mat4 world;

void main()
{
	vec4 pos = vec4(position, 1.0);
	gl_Position = view_projection * world * pos;
}
