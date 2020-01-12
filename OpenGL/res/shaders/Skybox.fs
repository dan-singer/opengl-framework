#version 330 core

// Note that we're using a vec3 because it's a cube map
in vec3 v_TexCoords;

uniform samplerCube cubemap;

out vec4 FragColor;

void main()
{
    FragColor = texture(cubemap, v_TexCoords);
}