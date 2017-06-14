#version 330 core
layout (location = 0) in vec3 position;

out vec3 corDinamica;
uniform vec3 cor;
uniform mat4 transforma;

void main()
{
    gl_Position = transforma*vec4(position, 1.0f);
    corDinamica = cor;
    
}