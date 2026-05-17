#version 450 core
out vec4 FragColor;
 
in vec4 Position;
in vec4 WorldPosition;

void main()
{
	FragColor = vec4((abs(WorldPosition.xyz/10.0))*0.4, 1.0);
}