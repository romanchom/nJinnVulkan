#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;

layout (location = 0) out vec3 diffuse;
layout (location = 1) out vec3 normal;

layout (set = 1, binding = 0) uniform asd{
	vec4 someUniform;
};

void main() 
{
	gl_Position = vec4(inPos * 0.1 + someUniform.xyz, 1.0);

	diffuse = vec3(inUv, 0);
	normal = inNormal;
}

