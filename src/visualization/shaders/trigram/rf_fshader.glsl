#version 330
in vec3 int_color;
in float cross_coord;
in float thru_coord;
out vec4 out_color;
void main() {
  float intensity = 1.0 - sqrt(cross_coord * cross_coord + thru_coord * thru_coord);
  out_color = vec4(int_color * intensity, 0);
}
