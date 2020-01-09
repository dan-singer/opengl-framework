#version 330 core
in vec2 v_TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0 - texture(screenTexture, v_TexCoords).xyz, 1.0);
}