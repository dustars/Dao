#version 450 core

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjMatrix;

layout(location = 0) in vec3 position;
layout(location = 2) in vec2 texCoord;

out VS{
	vec2 texCoord;
}OUT;

void main(void) {
	gl_Position = ProjMatrix * ViewMatrix * ModelMatrix * vec4(position, 1.0f);
	OUT.texCoord = texCoord;
}