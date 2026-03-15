#version 330 core

layout(location = 0) in vec3 a_model_vertex_pos;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec3 a_position;
layout(location = 3) in float a_radius;
layout(location = 4) in vec3 a_color;

out vec3 o_vertex_color;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    // scale and shift
    vec3 real_pos = (a_model_vertex_pos * a_radius) + a_position;

    gl_Position = u_projection * u_view * u_model * vec4(real_pos, 1.0);
    o_vertex_color = a_color;
}
