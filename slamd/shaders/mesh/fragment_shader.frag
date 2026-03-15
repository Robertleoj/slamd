#version 330 core

in vec3 Normal;
in vec3 vertex_color;

out vec4 FragColor;

uniform mat4 view;
uniform float min_brightness;
uniform float alpha;

void main() {
    vec3 norm = normalize(Normal);

    // Camera-relative lighting: extract camera forward and right from view matrix
    vec3 cam_forward = normalize(vec3(view[0][2], view[1][2], view[2][2]));
    vec3 cam_right = normalize(vec3(view[0][0], view[1][0], view[2][0]));

    // Key light: slightly offset from camera direction
    vec3 key_dir = normalize(cam_forward + 0.3 * cam_right);
    float key = max(dot(norm, key_dir), 0.0);

    // Fill light: from the opposite side, softer
    vec3 fill_dir = normalize(cam_forward - 0.5 * cam_right);
    float fill = max(dot(norm, fill_dir), 0.0) * 0.4;

    // Blinn-Phong specular (using camera forward as view direction)
    vec3 half_dir = normalize(key_dir + cam_forward);
    float spec = pow(max(dot(norm, half_dir), 0.0), 32.0) * 0.3;

    float ambient = min_brightness;
    float lighting = ambient + (1.0 - ambient) * (key + fill);
    lighting = min(lighting, 1.0);

    FragColor = vec4(vertex_color * lighting + vec3(spec), alpha);
}