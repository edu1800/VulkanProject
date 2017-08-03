#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) out vec4 color;
//layout(location = 0) out float fragmentdepth;

layout(std140, push_constant) uniform cameraBlock {
    vec2 cameraClip;
} cameraConstantsBlock;

float LinearizeDepth(float depth, float n, float f)
{
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main() 
{	
//	fragmentdepth = gl_FragCoord.z;
	//color = vec4(vec3(1.0-LinearizeDepth(gl_FragCoord.z, cameraConstantsBlock.cameraClip.x, cameraConstantsBlock.cameraClip.y)), 1.0);
	color = vec4(gl_FragCoord.z, gl_FragCoord.z, gl_FragCoord.z, 1);
}