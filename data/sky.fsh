#version 330 core

in vec2 v_texcoord; // texture coords
in vec3 v_normal;   // normal
in vec3 v_binormal; // binormal (for TBN basis calc)
in vec3 v_pos;      // pixel view space position

out vec4 color;

uniform sampler2D tex;     // base texture (albedo)

const vec3 light_dir = normalize(vec3(0.0, 1.0, 1.0));

void main()
{
    float l = (dot(light_dir, v_normal) + 1.0) / 2.0;

    vec3 dark_blue = vec3(4.0 / 255.0, 39.0 / 255.0, 181.0 / 255.0);
    vec3 light_blue = vec3(131.0 / 255.0, 187.0 / 255.0, 248.0 / 255.0);



    color = vec4(mix(dark_blue, light_blue, l * 2.0), 1.0);
}
