#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;

uniform mat4 view_projection;
uniform mat4 world;

void main()
{
	//gl_Position = view_projection * world * vec4(position, 1);
	vec4 pos = vec4(position, 1.0);
	pos.z = 0.0;
	gl_Position = pos;
}
