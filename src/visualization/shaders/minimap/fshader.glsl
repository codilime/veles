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
uniform float top_line_pos;
uniform float bottom_line_pos;
uniform int channel;
void main() {
    vec4 t = texture(tx, v_coord);
    float bgcolor = t.x / 255.0;
    if (v_coord.y < bottom_line_pos && v_coord.y > top_line_pos) {
        bgcolor = 0.0;
    }
    o_color = vec4(bgcolor, bgcolor, bgcolor, 0);
    o_color[channel] = t.x / 255.0;
}
