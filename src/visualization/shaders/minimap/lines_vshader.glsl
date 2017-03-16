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
uniform vec2 coords[4];
uniform vec2 tcoords[4];
out vec2 v_coord;

void main() {
	int vid = gl_VertexID;
    gl_Position = vec4(coords[vid], 1, 1);
    v_coord = tcoords[vid];
}
