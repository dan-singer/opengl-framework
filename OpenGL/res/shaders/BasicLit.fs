#version 330 core

in vec3 v_Normal;
in vec3 v_FragPos;

uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPos;
uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Ambient
    float ambientStrength = 0.1;
    vec3 ambient = lightColor * ambientStrength;

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 toLight = normalize(lightPos - v_FragPos);
    float diffStrength = max(dot(norm, toLight), 0);
    vec3 diffuse = lightColor * diffStrength;

    // Specular
    float specularStrength = 0.5;
    float shininess = 32;
    vec3 toViewer = normalize(viewPos - v_FragPos);
    /*
    https://learnopengl.com/Lighting/Basic-Lighting
    Note that we negate the lightDir vector.
    The reflect function expects the first vector to point from the light source
    towards the fragment's position,but the lightDir vector is currently pointing
    the other way around from the fragment towards the light source
    (depends on the order of subtraction earlier on when we calculated the lightDir vector).
    To make sure we get the correct reflect vector we reverse its direction by negating the 
    lightDir vector first. The second argument expects a normal vector so we supply the 
    normalized norm vector.*/
    vec3 reflectDir = reflect(-toLight, norm);
    float spec = pow(max(dot(toViewer, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor; 

    // Combine
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}