//[VERTEX SHADER]
#version 130

uniform mat4 proj;
uniform mat4 model;
in vec3 position;
void main()
{
  gl_Position = proj * model * vec4(position,1.);
}