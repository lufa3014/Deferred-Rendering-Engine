#version 410 core

/**
 * Directional Light Shadow Shader.
 *
 * Copyright (C) 2020, FH Wedel
 * Autor: stud105751, stud104645
 */

layout (location = 0) in vec3 aPos;

uniform mat4 u_model;
uniform mat4 u_lightSpace;

void main()
{
    //https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
    gl_Position = u_lightSpace * u_model * vec4(aPos, 1.0);
}
