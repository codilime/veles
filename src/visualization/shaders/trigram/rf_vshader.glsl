#version 330

in vec3 vert;
in vec3 color;
out vec3 v2g_color;
out vec4 v2g_position;
uniform mat4 mvp;

vec3 apply_coord_system(vec3 vert);

void main() {
  v2g_position = mvp * vec4(apply_coord_system(vert), 1.0);
  v2g_color = color;
}
