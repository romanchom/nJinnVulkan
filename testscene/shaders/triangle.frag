#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 color;
layout (location = 1) out vec4 color2;

void main() 
{
	color = vec4(inColor, 1);
	color2 = vec4(1 - inColor, 1);
}