#version 330 core

in vec2 v_texcoord; // texture coords
in vec3 v_normal;   // normal
in vec3 v_binormal; // binormal (for TBN basis calc)
in vec3 v_pos;      // pixel view space position

out vec4 color;

uniform sampler2D tex;     // base texture (albedo)

const vec3 light_dir = vec3(0.0, 1.0, 1.0);

void main()
{
    color = vec4((v_normal + 1.0) * 0.5, 1.0);
}
