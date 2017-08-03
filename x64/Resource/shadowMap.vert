#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (location = 0) in vec3 inPos;

layout (binding = 0) uniform UBO 
{
	mat4 localToWorld;
    mat4 viewMatrix;
    mat4 projMatrix;
	vec3 cameraPos;
} ubuf;

out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	gl_Position = ubuf.projMatrix * ubuf.viewMatrix * ubuf.localToWorld * vec4(inPos, 1.0);
}