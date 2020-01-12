#version 330 core

layout (location = 0) in vec3 position;

out vec3 v_TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    v_TexCoords = position;
    vec4 pos = projection * view * vec4(position, 1.0);
    gl_Position = pos.xyww; // Make the depth buffer always have a value of 1.0!
}