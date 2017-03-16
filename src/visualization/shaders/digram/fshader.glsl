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
in vec2 v_coord;
layout (location = 0, index = 0) out vec4 o_color;
uniform sampler2D tx;
void main() {
	vec4 t = texture(tx, v_coord);
	float clr = t.x;
	float ch = t.y;
	ch /= clr;
	clr *= 4096.0;
//	if (clr != 0.0)
//		clr = 1.0 + (log(clr) / 10.0);
	o_color = vec4(clr * (1.0 - ch), clr/2.0, clr * ch, 0);
}
