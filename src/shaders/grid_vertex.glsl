#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

uniform mat4 view;
uniform mat4 projection;

out vec3 fColor;
out vec3 fragPos;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    fColor = aColor;
    fragPos = vec3(aPos);
}  