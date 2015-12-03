//[VERTEX SHADER]
#version 130

uniform mat4 model;
uniform mat4 proj;

in vec3 position;
out vec3 normal;
out vec3 pos;

varying vec4 n;
void main()
{
	n = gl_ModelViewProjectionMatrix * vec4(gl_Normal,0);
	normal = vec3(n.x,n.y,n.z);
  	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; 
  	pos = gl_Position.xyz;
}