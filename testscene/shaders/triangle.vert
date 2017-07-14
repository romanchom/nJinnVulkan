#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// vertex attributes
layout (location = 0)
in vec3 inPos;

layout (location = 1)
in vec2 inUv;

layout (location = 2)
in vec3 inNormal;

layout (location = 3)
in vec3 inTangent;


// outputs
layout (location = 0)
out vec3 outNormal;

layout (location = 1)
out vec3 outTangent;

layout (location = 2)
out vec2 outUv;

// uniforms
layout (set = 0, binding = 0) uniform a {
	mat4 modelViewProjection;
	mat4 modelViewInverseTransposed;
};

layout (set = 2, binding = 0) uniform b {
	mat4 model;
};

void main() {
	gl_Position = modelViewProjection * vec4(inPos, 1.0);
	outNormal = (modelViewInverseTransposed * vec4(inNormal, 0.0)).xyz;
	outTangent = (modelViewInverseTransposed * vec4(inTangent, 0.0)).xyz;
}
