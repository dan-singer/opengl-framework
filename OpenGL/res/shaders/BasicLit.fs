#version 330 core

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoords;

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    sampler2D emission;
    float shininess;
};
uniform Material material;

struct Light {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform Light light;

uniform vec3 viewPos;

out vec4 FragColor;

void main()
{
    // Ambient
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, v_TexCoords));

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 toLight = normalize(light.position - v_FragPos);
    float diffStrength = max(dot(norm, toLight), 0);
    vec3 diffuse = light.diffuse * (diffStrength * vec3(texture(material.diffuse, v_TexCoords)));

    // Specular
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
    float spec = pow(max(dot(toViewer, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.specular, v_TexCoords))); 

    // Emission
    vec3 emissive = vec3(texture(material.emission, v_TexCoords));

    // Combine
    vec3 result = ambient + diffuse + specular + emissive;
    FragColor = vec4(result, 1.0);
}