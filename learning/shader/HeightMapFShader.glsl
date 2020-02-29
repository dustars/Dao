#version 450 core

uniform int UseTexture = 0;
uniform sampler2D ourTexture;

in VS{
	vec4 color;
	vec2 texCoord;
}IN;

out vec4 FragColor;

void main()
{
	FragColor = texture(ourTexture, IN.texCoord) * IN.color;
}