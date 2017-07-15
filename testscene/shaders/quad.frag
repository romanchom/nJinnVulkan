#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable



layout (set = 0, binding = 0, input_attachment_index = 0)
uniform subpassInput depthSubpass;

layout (set = 0, binding = 1, input_attachment_index = 1)
uniform subpassInput diffuseSubpass;

layout (set = 0, binding = 2, input_attachment_index = 2)
uniform subpassInput normalSubpass;

layout (location = 0) out vec4 color;


vec3 decodeNormal() {
	vec2 enc = subpassLoad(normalSubpass).xy * 2;
	float l = dot(enc, enc) * 0.25;
	float s = sqrt(1 - l);
	return vec3(enc.xy * s, 1 - 2 * l);
}

void main() 
{
	//color = subpassLoad(diffuseSubpass);
	vec3 normal = decodeNormal();
	
	color = vec4(dot(normal, normalize(vec3(1))));
	//color = vec4(normal, 1);
	//color = vec4(subpassLoad(normalSubpass).xy, 0, 1);
}