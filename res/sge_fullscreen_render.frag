#version 450

layout (binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	vec2 f = vec2 (inUV.s, 1.0 - inUV.t);
  	outFragColor = texture(samplerColor, f);
}