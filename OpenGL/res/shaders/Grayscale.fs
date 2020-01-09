#version 330 core

in vec2 v_TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main()
{
    FragColor = texture(screenTexture, v_TexCoords);
    // https://learnopengl.com/Advanced-OpenGL/Framebuffers
    float average = 0.2126 * FragColor.r + 0.7152 * FragColor.g + 0.0722 * FragColor.b;
    FragColor = vec4(average, average, average, 1.0);
}