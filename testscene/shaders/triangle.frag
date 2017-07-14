#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 encodeNormal(vec3 normal) {
	return normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
}

// inputs
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec2 inUv;

layout (location = 0) out vec4 outDiffuse;
layout (location = 1) out vec4 outNormal;

void main() 
{
	outNormal = vec4(0);
	outNormal.xy = encodeNormal(inNormal);
	outDiffuse = vec4(1, 0, 1, 1);
	outDiffuse = vec4(inNormal, 1);
}