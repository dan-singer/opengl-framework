#version 330 core

in vec3 v_Normal;
in vec3 v_FragPos;
in vec2 v_TexCoords;

struct Material {
    sampler2D texture_diffuse0;
    sampler2D texture_specular0;
    float shininess;
};
uniform Material material;

struct DirectionalLight {
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};
uniform DirectionalLight directionalLight;

struct PointLight {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float constant;
    float linear;
    float quadratic;
};
#define NUM_POINT_LIGHTS 4
uniform PointLight pointLights[NUM_POINT_LIGHTS];

struct SpotLight {
    vec3 position;
    vec3 direction;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outerCutoff;
};
uniform SpotLight spotLight;
uniform vec3 viewPos;

out vec4 FragColor;


vec3 CalcDirectionalLight(DirectionalLight light) 
{
    vec3 rawDiffuseColor = vec3(texture(material.texture_diffuse0, v_TexCoords));

    // Ambient
    vec3 ambient = light.ambient * rawDiffuseColor;

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 toLight = normalize(-light.direction);
    float diffStrength = max(dot(norm, toLight), 0);
    vec3 diffuse = light.diffuse * (diffStrength * rawDiffuseColor);

    // Specular
    vec3 toViewer = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-toLight, norm);
    float spec = pow(max(dot(toViewer, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.texture_specular0, v_TexCoords)));

    // Combine
    vec3 result = ambient + diffuse + specular;
    return result;
}

vec3 CalcPointLight(PointLight light)
{
    vec3 rawDiffuseColor = vec3(texture(material.texture_diffuse0, v_TexCoords));

    // Ambient
    vec3 ambient = light.ambient * rawDiffuseColor;

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 toLight = normalize(light.position - v_FragPos);
    float diffStrength = max(dot(norm, toLight), 0);
    vec3 diffuse = light.diffuse * (diffStrength * rawDiffuseColor);

    // Specular
    vec3 toViewer = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-toLight, norm);
    float spec = pow(max(dot(toViewer, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.texture_specular0, v_TexCoords)));

    // Attenuation
    float dist = length(v_FragPos - light.position);
    float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * dist * dist);
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    // Combine
    vec3 result = ambient + diffuse + specular;
    return result;
}

vec3 CalcSpotLight(SpotLight light)
{
    vec3 rawDiffuseColor = vec3(texture(material.texture_diffuse0, v_TexCoords));

    // Ambient
    vec3 ambient = light.ambient * rawDiffuseColor;

    // Diffuse
    vec3 norm = normalize(v_Normal);
    vec3 toLight = normalize(light.position - v_FragPos);
    float diffStrength = max(dot(norm, toLight), 0);
    vec3 diffuse = light.diffuse * (diffStrength * rawDiffuseColor);

    // Specular
    vec3 toViewer = normalize(viewPos - v_FragPos);
    vec3 reflectDir = reflect(-toLight, norm);
    float spec = pow(max(dot(toViewer, reflectDir), 0.0), material.shininess);
    vec3 specular = light.specular * (spec * vec3(texture(material.texture_specular0, v_TexCoords)));

    // Spot-light intensity
    float cosTheta = dot(toLight, normalize(-light.direction));
    float epsilon = light.cutoff - light.outerCutoff;
    float intensity = clamp((cosTheta - light.outerCutoff) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;

    // Combine
    vec3 result = ambient + diffuse + specular;
    return result;
}

void main()
{
    // Directional Light (just one for now!)
    vec3 result = CalcDirectionalLight(directionalLight);
    // Point Lights
    for (int i = 0; i < NUM_POINT_LIGHTS; ++i) 
    {
        result += CalcPointLight(pointLights[i]);
    }
    // Spot light (just one for now!)
    result += CalcSpotLight(spotLight);

    FragColor = vec4(result, 1.0);
}