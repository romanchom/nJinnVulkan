#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 encodeNormal(vec3 normal) {
	return normalize(normal.xy) * sqrt(normal.z * -0.5 + 0.5);
}

vec2 stereo(vec3 normal) {
	return normal.xy / (normal.z * -1.42 + 1.42);
}

vec2 lambert(vec3 normal){
	float a = sqrt(2 / (1 + normal.z)) / 2;
	return normal.xy * a;
}

// inputs
layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec2 inUv;

layout (location = 0) out vec4 outDiffuse;
layout (location = 1) out vec4 outNormal;

void main() 
{
	outNormal.xy = lambert(normalize(inNormal));
	outDiffuse = vec4(1, 0, 1, 1);
}