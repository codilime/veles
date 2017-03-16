/*
 * Copyright 2016 CodiLime
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
#version 330

uniform mat4 xfrm;
uniform usamplerBuffer tx;
uniform uint sz;
uniform float c_cyl, c_sph, c_pos;
out float v_pos;
const float TAU = 3.1415926535897932384626433832795 * 2;

void main() {
	int vid = gl_VertexID;
	v_pos = float(vid) / float(sz - 3u);
	uint x = texelFetch(tx, vid).x;
	uint y = texelFetch(tx, vid + 1).x;
	uint z = texelFetch(tx, vid + 2).x;
	vec3 v_coord = vec3(float(x)+0.5, float(y)+0.5, float(z)+0.5) / 256.0;
	v_coord.z *= (1.0 - c_pos);
	v_coord.z += c_pos * v_pos;
	vec3 xpos = v_coord * vec3(2, 2, 2) - vec3(1, 1, 1);
	xpos *= (1.0 - c_cyl - c_sph);
	vec2 a1pos = vec2(cos(v_coord.x * TAU), sin(v_coord.x * TAU));
	vec2 a2pos = vec2(sin(v_coord.y * TAU / 2.0), cos(v_coord.y * TAU / 2.0));
	vec3 cpos = vec3(a1pos * v_coord.y, v_coord.z * 2.0 - 1.0);
	xpos += cpos * c_cyl;
	vec3 spos = vec3(a1pos * a2pos.x, a2pos.y) * v_coord.z;
	xpos += spos * c_sph;
	gl_Position = xfrm * vec4(xpos, 1);
}
