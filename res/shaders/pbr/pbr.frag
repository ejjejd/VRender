#version 460 core

#define MAX_POINT_LIGHTS 32
#define MAX_SPOTLIGHTS 32

#define PI 3.14159265359f

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec3 Camera;

struct PointLight
{
	vec4 Position;
	vec4 Color;
};

struct Spotlight
{
	vec4 Position;
	vec4 Direction;
	vec4 Color;

	float OuterAngle;
	float InnerAngle;
};

layout(set = 0, binding = 1) uniform LightUBO
{
	PointLight PointLights[MAX_POINT_LIGHTS + 1];
	Spotlight Spotlights[MAX_SPOTLIGHTS + 1];
} lightUBO;

layout(set = 2, binding = 0) uniform MaterialUBO
{
	vec3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;
} materialUBO;

vec3 FresnelSchlick(float theta, vec3 F0)
{
	return F0 + (1.0f - F0) * pow(max(1.0f - theta, 0.0f), 5.0f);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a = roughness * roughness;
	float a2 = a * a;
	float NdotH = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float num = a2;
	float denom = (NdotH2 * (a2 - 1.0f) + 1.0f);
	denom = PI * denom * denom;

	return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;

	float num = NdotV;
	float denom = NdotV * (1.0f - k) + k;

	return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);

	float ggx2 = GeometrySchlickGGX(NdotV, roughness);
	float ggx1 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

void main()
{
	vec3 lightPos = lightUBO.PointLights[0].Position.xyz;
	vec3 lightColor = lightUBO.PointLights[0].Color.xyz;

	vec3 N = normalize(Normal);
	vec3 V = normalize(Camera - FragPos);

	vec3 Lo = vec3(0.0f);

	vec3 L = normalize(lightPos - FragPos);
	vec3 H = normalize(L + V);

	float distance = length(lightPos - FragPos);
	float attenuation = 1.0f / (distance * distance);
	vec3 radiance = lightColor * attenuation;

	vec3 F0 = vec3(0.04f);
	F0 = mix(F0, materialUBO.Albedo, materialUBO.Metallic);
	vec3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0);

	float NDF = DistributionGGX(N, H, materialUBO.Roughness);
	float G = GeometrySmith(N, V, L, materialUBO.Roughness);

	vec3 numerator = NDF * G * F;
	float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);

	vec3 specular = numerator / max(denom, 0.001f);

	vec3 kS = F;
	vec3 kD = vec3(1.0f) - kS;

	kD *= 1.0f - materialUBO.Metallic;

	float NdotL = max(dot(N, L), 0.0f);
	Lo = (kD * materialUBO.Albedo / PI + specular) * radiance * NdotL;

	vec3 ambient = vec3(0.03f) * materialUBO.Albedo * materialUBO.Ao;
	vec3 color = ambient + Lo;

	outColor = vec4(color, 1.0f);
}