#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
layout (std140, binding = 0) uniform buf {
        mat4 localToWorld;
        mat4 viewMatrix;
        mat4 projMatrix;
		mat4 lightMatrix;
        vec3 camWorldPos;
} ubuf;
layout(std140, push_constant) uniform lightBlock {
    layout(offset = 0) vec3 LightPosition;
    layout(offset = 16) vec3 LightDirection;
    layout(offset = 32) vec3 LightColor;
    layout(offset = 48) vec3 LightIntensity;
} lightConstantsBlock;
layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 inTexCoords;
layout (location = 0) out vec2 texcoord;
layout (location = 1) out vec4 worldPos;
layout (location = 2) out vec3 lightDir;
layout (location = 3) out vec3 lightColor;
layout (location = 4) out vec3 outNormal;
layout (location = 5) out vec3 camPos;
layout (location = 6) out vec4 shadowCoord;
out gl_PerVertex { 
    vec4 gl_Position;
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main() {
   texcoord = inTexCoords;
	worldPos = ubuf.localToWorld * vec4(pos, 1.0);
	lightDir = -lightConstantsBlock.LightDirection;
	lightColor = lightConstantsBlock.LightColor * lightConstantsBlock.LightIntensity.r;
   gl_Position = ubuf.projMatrix * ubuf.viewMatrix * worldPos;
	outNormal = mat3(inverse(transpose(ubuf.localToWorld))) * normal;
	camPos = ubuf.camWorldPos;
	shadowCoord = biasMat * ubuf.lightMatrix * worldPos;
}