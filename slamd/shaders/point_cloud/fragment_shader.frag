#version 330 core

in vec3 o_vertex_color;

out vec4 FragColor;

void main() {
    FragColor = vec4(o_vertex_color, 1.0);
}