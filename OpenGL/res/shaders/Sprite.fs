#version 330 core
in vec2 v_TexCoords;

uniform sampler2D diffuse;
out vec4 FragColor;

void main()
{
    FragColor = texture(diffuse, v_TexCoords);
}