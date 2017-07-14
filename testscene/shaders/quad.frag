#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

vec2 encodeNormal(vec3 normal) {
	return normalize(normal.xy) * sqrt(normal.z * 0.5 + 0.5);
}

vec3 decodeNormal(vec2 encoded) {
	vec3 ret;
	ret.z = dot(encoded, encoded) * 2 - 1;
	ret.xy = normalize(encoded) * sqrt(1 - ret.z * ret.z);
	return ret;
}


layout (set = 0, binding = 0, input_attachment_index = 0)
uniform subpassInput depthSubpass;

layout (set = 0, binding = 1, input_attachment_index = 1)
uniform subpassInput diffuseSubpass;

layout (set = 0, binding = 2, input_attachment_index = 2)
uniform subpassInput normalSubpass;

layout (location = 0) out vec4 color;

void main() 
{
	//color = subpassLoad(diffuseSubpass);
	//color = vec4(decodeNormal(subpassLoad(normalSubpass).xy), 1);
	color = subpassLoad(diffuseSubpass);
}