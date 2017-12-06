#version 330
in vec3 vert;
in vec3 color;
out vec3 int_color;
uniform mat4 mvp;

vec3 apply_coord_system(vec3 vert);

void main() {
  gl_Position = mvp * vec4(apply_coord_system(vert), 1.0);
  int_color = color;
}
