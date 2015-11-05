//[FRAGMENT SHADER]
#version 130
precision highp float;

in vec3 normal;

out vec3 color;
void main()
{
  color = normal;
}
