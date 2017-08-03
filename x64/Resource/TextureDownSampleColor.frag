#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	ivec2 texDim = textureSize(samplerColor, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	vec4 color = vec4(0, 0, 0, 0);
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			color += texture(samplerColor, inUV + vec2(dx*x, dy*y));
			count++;
		}
	
	}
	
	vec4 s = color / count;
	outFragColor = s;
}