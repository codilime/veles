#version 330
uniform sampler2D texture_sampler;
in vec2 tex_coord_out;
out vec4 out_color;
void main() {
  out_color = texture(texture_sampler, tex_coord_out);
}
