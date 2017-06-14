#version 330 core

in vec3 corDinamica;
out vec4 color;

void main()
{
	color = vec4(corDinamica,1.0f);

}