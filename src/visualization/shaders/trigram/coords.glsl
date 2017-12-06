#version 330

uniform float c_cyl, c_sph;
const float TAU = 3.1415926535897932384626433832795 * 2;

/*
 * Projects trigram vertex from normalized coordinates
 * in [0, 1] range via interpolated cube / cylinder / sphere coordinate
 * system to [-1, 1] range.
 */

vec3 apply_coord_system(vec3 vert) {
  vec3 cube_pos = vert * vec3(2, 2, 2) - vec3(1, 1, 1);
  /* Angle 1 -> circle point coordinates (for cylinder & sphere) */
  vec2 a1_pos = vec2(cos(vert.x * TAU), sin(vert.x * TAU));
  /* Angle 2 -> half-circle point coordinates (for sphere) */
  vec2 a2_pos = vec2(sin(vert.y * TAU / 2.0), cos(vert.y * TAU / 2.0));
  vec3 cylinder_pos = vec3(a1_pos * vert.y, vert.z * 2.0 - 1.0);
  vec3 sphere_pos = vec3(a1_pos * a2_pos.x, a2_pos.y) * vert.z;
  /* Interpolate */
  vec3 final_pos = cube_pos * (1.0 - c_cyl - c_sph);
  final_pos += cylinder_pos * c_cyl;
  final_pos += sphere_pos * c_sph;
  return final_pos;
}
