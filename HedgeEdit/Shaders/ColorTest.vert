#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 pos;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    pos = gl_Position;
}