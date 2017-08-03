#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout(std140, push_constant) uniform renderBlock {

	layout(offset = 64) vec3 Ka;

	layout(offset = 80) vec3 Kd;

	layout(offset = 96) vec3 Ks;

	layout(offset = 112) vec3 ns;

} renderConstantsBlock;

layout (binding = 1) uniform sampler2D tex;
layout (binding = 2) uniform sampler2D normalMap;
layout (binding = 3) uniform sampler2D shadowMap;
layout (location = 0) in vec2 texcoord;
layout (location = 1) in vec4 worldPos;
layout (location = 2) in vec3 lightDir;
layout (location = 3) in vec3 lightColor;
layout (location = 4) in vec3 normal;
layout (location = 5) in vec3 camPos;
layout (location = 6) in vec4 shadowCoord;
layout (location = 0) out vec4 outColor;

float textureProj(vec4 P, vec2 off)
{
	float shadow = 1.0;
	vec4 coord = P / P.w;
	if ( coord.z > -1.0 && coord.z < 1.0 ) 
	{
		float dist = texture( shadowMap, coord.st + off ).r;
		if ( dist < coord.z ) 
		{
			shadow = 0.1;
		}
	}
	return shadow;
}

vec3 filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(shadowMap, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	float s = shadowFactor / count;
	return vec3(s, s, s);
}

void main() {
	vec3 lightDir_n = normalize(lightDir);
	vec3 viewDir = normalize(camPos - worldPos.xyz);
	vec3 normal_n = normalize( 2 * (texture(normalMap, texcoord).xyz - 0.5));
	vec3 h = lightDir_n + viewDir;
	float diffuse = max(dot(normal_n, lightDir_n), 0.1);
	float nh = clamp(dot(h, normal_n), 0.0, 1.0);
	float spec = pow(nh, renderConstantsBlock.ns.r);
	vec4 col = texture(tex, texcoord);
	vec3 shadow = filterPCF(shadowCoord);
	outColor.rgb = col.rgb * renderConstantsBlock.Ka + col.rgb * lightColor * diffuse * renderConstantsBlock.Kd * shadow + vec3(1, 1, 1) * spec * renderConstantsBlock.Ks * shadow;
	outColor.a = col.a;
	//outColor.rgb = shadow;
}