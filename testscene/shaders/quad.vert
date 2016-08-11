#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (location = 0) out vec3 color;

layout (set = 1, binding = 0) uniform asd{
	vec4 someUniform;
};

void main() 
{
	gl_Position = vec4(inPos, 1.0);
	color = vec3(inPos.xy * 0.5 + 0.5, 0);
}
