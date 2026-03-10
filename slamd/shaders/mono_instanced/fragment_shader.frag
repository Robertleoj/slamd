#version 330 core

in vec3 o_normal;
in vec3 o_vertex_color;

out vec4 FragColor;

uniform mat4 u_view;
uniform float u_min_brightness;

void main() {
    vec3 norm = normalize(o_normal);

    vec3 cam_forward = normalize(vec3(u_view[0][2], u_view[1][2], u_view[2][2]));
    vec3 cam_right = normalize(vec3(u_view[0][0], u_view[1][0], u_view[2][0]));

    vec3 key_dir = normalize(cam_forward + 0.3 * cam_right);
    float key = max(dot(norm, key_dir), 0.0);

    vec3 fill_dir = normalize(cam_forward - 0.5 * cam_right);
    float fill = max(dot(norm, fill_dir), 0.0) * 0.4;

    // Blinn-Phong specular (using camera forward as view direction)
    vec3 half_dir = normalize(key_dir + cam_forward);
    float spec = pow(max(dot(norm, half_dir), 0.0), 32.0) * 0.3;

    float ambient = u_min_brightness;
    float lighting = ambient + (1.0 - ambient) * (key + fill);
    lighting = min(lighting, 1.0);

    FragColor = vec4(o_vertex_color * lighting + vec3(spec), 1.0);
}