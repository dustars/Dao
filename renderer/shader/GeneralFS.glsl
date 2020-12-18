#version 450 core

layout (binding = 0) uniform sampler2D ourTexture;
layout (binding = 2) uniform sampler2D shadowMap;

layout (binding = 10) uniform sampler2D tex1;
layout (binding = 11) uniform sampler2D tex2;
layout (binding = 12) uniform sampler2D tex3;
layout (binding = 13) uniform sampler2D tex4;

layout(binding = 0, offset = 0) uniform atomic_uint counter;

uniform vec3 cameraPos;
uniform vec4 LightPos;
uniform vec4 LightColor;
uniform vec3 sunDir;

in VS {
	vec3 color;
	vec2 texCoord;
	vec3 vPos;
	vec3 vNormal;
	vec4 lightSpacePos;
} IN;

out vec4 FragColor;

float ShadowCalculation() {
	vec3 projPos = IN.lightSpacePos.xyz / IN.lightSpacePos.w;
	projPos = projPos * 0.5 + 0.5;
	float shadowDepth = texture(shadowMap, projPos.xy).r;
	float projDepth = projPos.z;
	return projDepth > shadowDepth + 0.000005 ? 0.0 : 1.0;
}

void main() {
	//vec4 diffuse = texture(ourTexture, IN.texCoord) * vec4(IN.color, 1.f) * LightColor;
	vec4 diffuse = texture(tex1, IN.texCoord) * LightColor;
	//vec4 diffuse = vec4(1, 1, 1, 1);// *LightColor;

	vec3 n = normalize(IN.vNormal);
	vec3 v = normalize(cameraPos - IN.vPos);
	//The sun direction doesn't have shadow mappings, unfortunately
	//vec3 l = normalize(LightPos.xyz - IN.vPos);
	vec3 l = sunDir;
	vec3 halfDir = normalize(l + v);

	//How much diffuse reflection
	float lambert = clamp(dot(n, l), 0.0, 1.0);
	//How much specular reflection
	float rFactor = clamp(dot(halfDir, n), 0.0, 1.0);
	float sFactor = pow(rFactor, 64.0); // configurable

	float shadow = ShadowCalculation();

	vec3 color	=  diffuse.rgb;							// diffuse
		 color	+= (LightColor.rgb * sFactor) * 0.4;	// specularity, the right number is configurable
		 color	*= lambert;								// No attenuation for now
		 color  *= shadow;								// Shadow
		 color	+= diffuse.rgb * 0.1;					// ambience;
	
	FragColor = vec4(color, diffuse.a);
	//uint counter = atomicCounterIncrement(counter);
	//float r = (counter / 1024) / 1024.f;
	//FragColor = vec4(r, 0, 0, 1);
}