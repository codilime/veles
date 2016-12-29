#version 330
in vec3 vert;
in vec2 tex_coord_in;
uniform mat4 mvp;
out vec2 tex_coord_out;
void main() {
  gl_Position = mvp * vec4(vert, 1.0);
  tex_coord_out = tex_coord_in;
}
