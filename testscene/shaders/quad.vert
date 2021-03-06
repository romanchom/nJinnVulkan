#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// vertex attributes
layout (location = 0) in 
vec3 inPos;

void main() 
{
	gl_Position = vec4(inPos, 1.0);
}
