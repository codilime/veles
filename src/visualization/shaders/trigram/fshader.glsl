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

in float v_pos;
in float v_factor;
layout (location = 0, index = 0) out vec4 o_color;
uniform float c_brightness;
void main() {
	vec2 diff = vec2(dFdx(gl_PointCoord.x), dFdy(gl_PointCoord.y));
	float cdist = length(gl_PointCoord.xy - vec2(0.5, 0.5)) - length(diff) / 2;
	float maxdist = max(0.5 - length(diff) / 2, 0.1);
	float cscale = clamp((maxdist - cdist)/maxdist, 0.0, 1.0);
	o_color = vec4(1.0 - v_pos, 0.5, v_pos, 1) * c_brightness * cscale * v_factor;
}
