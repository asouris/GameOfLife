#version 330 core
out vec4 FragColor;
  
in vec3 fColor;
in vec3 fragPos;
in vec3 fragNormal;

uniform vec3 uniColor;
uniform float light_intensity;

struct DirectionalLight
{
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
} light;

vec3 computeDirectionalLight() {
    light.direction = vec3(-1, 1, 1);
    light.ambient = vec3(0.5, 0.5, 0.5);
    light.diffuse = vec3(1, 1, 1);

    //ambient
    vec3 ambient = light.ambient * uniColor;

    // diffuse
    float diff = max(dot(normalize(fragNormal), light.direction), 0.0f);
    vec3 diffuse = uniColor * light.diffuse * diff;

    return ambient+diffuse;
}

void main()
{
    FragColor = vec4(computeDirectionalLight() * light_intensity, 1.0f);
}