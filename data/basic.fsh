#version 330 core

in vec3 v_normal;   // normal

out vec4 color;

void main()
{
    vec3 rgb = (v_normal + 1.0) / 2.0;
    color = vec4(rgb, 1.0);
}
