#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal; // While this is unused, it matches the Vertex struct
layout (location = 2) in vec2 texCoords;

out vec2 v_TexCoords;

void main()
{
    gl_Position = vec4(position.x, position.y, 0.0, 1.0);
    v_TexCoords = texCoords;
}