#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 tangent;
layout(location = 3) in vec3 texcoord;

out vec2 v_texcoord; // texture coords
out vec3 v_normal;   // normal
out vec3 v_binormal; // binormal (for TBN basis calc)
out vec3 v_pos;      // pixel view space position

uniform mat4 view_matrix;
uniform mat4 proj_matrix;
uniform mat3 normal_matrix;
uniform mat4 world_matrix;

void main()
{
	v_texcoord = texcoord.xy;
	v_normal   = normal_matrix * normal;
	vec3 tangent = normal_matrix * tangent;
	v_binormal = cross(v_normal, tangent);

	vec4 view_space = view_matrix * world_matrix * vec4(position, 1.0);
	gl_Position = proj_matrix * view_space;

	v_pos = view_space.xyz;
}
