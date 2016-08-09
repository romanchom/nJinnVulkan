#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index=0, set=1, binding=0) uniform subpassInput gBuffer[2];

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 color;


void main() 
{
	color = subpassLoad(gBuffer[1]) + vec4(inColor.xyz, 1);
	//color = vec4(inColor.xyz, 1);
}