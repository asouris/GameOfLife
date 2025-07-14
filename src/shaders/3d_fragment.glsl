#version 330 core
out vec4 FragColor;
  
in vec3 fragPos;
in vec3 fragNormal;

uniform vec3 uniColor;
uniform float light_intensity;
uniform bool coloring;

const int max_light_cells = 20;
uniform float light_cells[max_light_cells];
uniform float light_cells_intensity;
uniform int number_light_cells;

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

vec3 computeCellLight(vec3 cell_position){
    // attenuation
    vec3 lightVec = cell_position - fragPos;
    float distance = length(lightVec);
    float attenuation = 1.0f / ( 0.09 * distance + 0.032 * distance * distance + 1.5 ); // attenuation with constant values
    attenuation = max(0, 1 - distance * (1/0.4));
    // ambient
    vec3 ambient = vec3(0.5, 0.5, 0.5) * vec3(1, 1, 0);

    // diffuse
    vec3 lightDir = normalize(lightVec);
    float diff = max(dot(fragNormal, lightDir), 0.0f);
    vec3 diffuse = vec3(1, 1, 1) * (diff * vec3(1, 1, 0));

    return (ambient + diffuse) * attenuation * light_cells_intensity;

}

void main()
{   

    if(!coloring){
        vec3 light_from_cells = vec3(0.0);
        if(number_light_cells > 0){
            for(int i = 0; i < number_light_cells; i++){
                light_from_cells += computeCellLight(vec3(light_cells[i*3], light_cells[i*3 + 1], light_cells[i*3 + 2]));
            }
        }

        FragColor = vec4(computeDirectionalLight() * light_intensity + light_from_cells, 1.0f);
    }
    else{
        FragColor = vec4(fragNormal, 1.0f);
    }
    
}