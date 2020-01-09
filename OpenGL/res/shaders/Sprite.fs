#version 330 core
in vec2 v_TexCoords;

uniform sampler2D diffuse;
out vec4 FragColor;

void main()
{
    vec4 texColor = texture(diffuse, v_TexCoords);
    FragColor = texColor;
}