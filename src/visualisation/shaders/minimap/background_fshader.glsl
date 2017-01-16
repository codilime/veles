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
layout (location = 0, index = 0) out vec4 o_color;
uniform int bg_red;
uniform int bg_green;
uniform int bg_blue;

void main() {
    o_color = vec4(bg_red / 255.0, bg_green / 255.0, bg_blue / 255.0, 1);
}
