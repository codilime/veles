#version 330
in vec3 int_color;
out vec4 out_color;
void main() {
  out_color = vec4(int_color, 1.0);
}
