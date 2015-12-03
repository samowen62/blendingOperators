//[FRAGMENT SHADER]
#version 130
precision highp float;

in vec3 normal;
in vec3 pos;

out vec3 color;
void main()
{
	vec3 source = vec3( 1.0, 1.0, 1.0 );
	vec3 light = normalize(source - pos);

	float diffuseCoefficient = max(0.0, dot(light,normal));

	vec3 diffuse_color = diffuseCoefficient * vec3(0.0,0.4,0.6);
	color = diffuse_color + vec3(0.3, 0.3, 0.4);
}
