//[VERTEX SHADER]
#version 130

uniform mat4 model;
uniform mat4 proj;

in vec3 position;
in vec3 normal;

varying vec4 pos;
void main()
{
  gl_Position = proj * model * vec4(position,1.);
  //pos = proj * model * vec4(position,1.);
  //gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);

}