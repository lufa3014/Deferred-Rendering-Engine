#version 410 core

/**
 * Point Shadow Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

in vec4 FragPos;

uniform vec3 u_position;
uniform float u_zFar;

void main() {
    gl_FragDepth = length(FragPos.xyz - u_position) / u_zFar;
}
