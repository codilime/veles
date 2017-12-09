#version 330

layout(lines) in;
layout(triangle_strip, max_vertices=8) out;

in vec3 v2g_color[2];
in vec4 v2g_position[2];
out vec3 int_color;
out float cross_coord;
out float thru_coord;
uniform vec2 c_viewport_size;
const float c_line_width = 10.0;

void main() {
  vec2 pos_norm[2];
  pos_norm[0] = v2g_position[0].xy / v2g_position[0].w;
  pos_norm[1] = v2g_position[1].xy / v2g_position[1].w;
  vec2 direction = normalize((pos_norm[1] - pos_norm[0]) * c_viewport_size);
  if (v2g_position[0].w * v2g_position[1].w < 0.0)
    direction *= -1.0;
  vec2 delta_dir = vec2(direction.y, -direction.x);
  vec4 delta = vec4(delta_dir * c_line_width / c_viewport_size, 0, 0);
  vec4 xdelta = vec4(direction * c_line_width / c_viewport_size, 0, 0);
  gl_Position = v2g_position[0] + delta - xdelta;
  cross_coord = 1.0;
  thru_coord = -1.0;
  int_color = v2g_color[0];
  EmitVertex();
  gl_Position = v2g_position[0] - delta - xdelta;
  cross_coord = -1.0;
  thru_coord = -1.0;
  int_color = v2g_color[0];
  EmitVertex();
  gl_Position = v2g_position[0] + delta;
  cross_coord = 1.0;
  thru_coord = 0.0;
  int_color = v2g_color[0];
  EmitVertex();
  gl_Position = v2g_position[0] - delta;
  cross_coord = -1.0;
  thru_coord = 0.0;
  int_color = v2g_color[0];
  EmitVertex();
  gl_Position = v2g_position[1] + delta;
  cross_coord = 1.0;
  thru_coord = 0.0;
  int_color = v2g_color[1];
  EmitVertex();
  gl_Position = v2g_position[1] - delta;
  cross_coord = -1.0;
  thru_coord = 0.0;
  int_color = v2g_color[1];
  EmitVertex();
  gl_Position = v2g_position[1] + delta + xdelta;
  cross_coord = 1.0;
  thru_coord = 1.0;
  int_color = v2g_color[1];
  EmitVertex();
  gl_Position = v2g_position[1] - delta + xdelta;
  cross_coord = -1.0;
  thru_coord = 1.0;
  int_color = v2g_color[1];
  EmitVertex();
}
