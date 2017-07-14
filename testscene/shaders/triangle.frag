#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 encodeNormal(vec3 normal) {
	return normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
}

layout (location = 0) in vec3 inDiffuse;
//layout (location = 1) in vec3 inNormal;

layout (location = 0) out vec4 diffuse;
layout (location = 1) out vec4 normal;

void main() 
{
	normal = vec4(1);
	//normal.xy = encodeNormal(inNormal);
	diffuse = vec4(1, 0, 1, 1);
}