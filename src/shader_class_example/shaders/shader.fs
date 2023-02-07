#version 330 core

out vec4 FragColour;
in vec3 outputColour;

void main()
{
   FragColour = vec4(outputColour, 1.0);
};