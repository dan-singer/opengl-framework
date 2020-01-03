#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 v_Normal;
out vec3 v_FragPos;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1);
    v_FragPos = vec3(model * vec4(position, 1));
    // This is called a normal matrix - avoids non-uniform scale issues!
    v_Normal = mat3(transpose(inverse(model))) * normal; 
}