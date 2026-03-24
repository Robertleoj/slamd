#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D layer_texture;

void main() {
    FragColor = texture(layer_texture, uv);
}
