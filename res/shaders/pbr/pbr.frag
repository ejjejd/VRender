#version 460 core

#define MAX_POINT_LIGHTS 32
#define MAX_SPOTLIGHTS 32

#define PI 3.14159265359f

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec3 FragPos;
layout(location = 1) in vec3 Normal;
layout(location = 2) in vec2 UV;
layout(location = 3) in vec3 Camera;
layout(location = 4) in vec3 Tangent;
layout(location = 5) in vec3 Bitangent;

layout(set = 3, binding = 0) uniform sampler2D AlbedoTexture;
layout(set = 3, binding = 1) uniform sampler2D MetallicTexture;
layout(set = 3, binding = 2) uniform sampler2D RoughnessTexture;
layout(set = 3, binding = 3) uniform sampler2D AoTexture;
layout(set = 3, binding = 4) uniform sampler2D NormalTexture;
layout(set = 3, binding = 5) uniform samplerCube IrradianceMap;

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
	int PointLightsCount;
	int SpotlightsCount;
} lightUBO;

layout(set = 2, binding = 0) uniform MaterialUBO
{
	vec3 Albedo;
	float Metallic;
	float Roughness;
	float Ao;
} materialUBO;

vec3 FresnelSchlick(float theta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(max(1.0 - theta, 0.0), 5.0);
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
	vec3 N;

	if(textureSize(NormalTexture, 0).x > 1)
	{
		vec3 n = normalize(Normal);	

		vec3 t = normalize(Tangent);
		t = t - dot(n, t) * n;

		vec3 b = normalize(Bitangent);
		b = b - dot(n, b) * n - dot(t, b) * t;

		mat3 tbn = mat3(t, b, n);

		N = (mat3(t, b, n) * texture(NormalTexture, UV).xyz) * 2.0f - 1.0f;
	}
	else 
		N = normalize(Normal);

	vec3 V = normalize(Camera - FragPos);

	vec3 Lo = vec3(0.0f);

	vec3 Albedo = materialUBO.Albedo * texture(AlbedoTexture, UV).xyz;
	float Metallic = materialUBO.Metallic * texture(MetallicTexture, UV).b;
	float Roughness = materialUBO.Roughness * texture(RoughnessTexture, UV).g;
	float Ao = materialUBO.Ao * texture(AoTexture, UV).r;

	vec3 F0 = vec3(0.04f);

	for(int i = 0; i < min(lightUBO.PointLightsCount, MAX_POINT_LIGHTS); ++i)
	{
		vec3 lightPos = lightUBO.PointLights[i].Position.xyz;
		vec3 lightColor = lightUBO.PointLights[i].Color.xyz;

		vec3 L = normalize(lightPos - FragPos);
		vec3 H = normalize(L + V);

		float distance = length(lightPos - FragPos);
		float attenuation = 1.0f / (distance * distance);
		vec3 radiance = lightColor * attenuation;

		F0 = mix(F0, Albedo, Metallic);
		vec3 F = FresnelSchlick(max(dot(H, V), 0.0f), F0, Roughness);

		float NDF = DistributionGGX(N, H, Roughness);
		float G = GeometrySmith(N, V, L, Roughness);

		vec3 numerator = NDF * G * F;
		float denom = 4.0f * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);

		vec3 specular = numerator / max(denom, 0.001f);

		vec3 kS = F;
		vec3 kD = vec3(1.0f) - kS;

		kD *= 1.0f - Metallic;

		float NdotL = max(dot(N, L), 0.0f);
		Lo += (kD * Albedo / PI + specular) * radiance * NdotL;
	}

	vec3 kS = FresnelSchlick(max(dot(N, V), 0.0f), F0, Roughness);
	vec3 kD = vec3(1.0f) - kS;
	kD *= 1.0f - Metallic;

	vec3 irradiance = texture(IrradianceMap, N).rgb;
	vec3 diffuse = irradiance * Albedo;
	vec3 ambient = (kD * diffuse) * Ao;

	vec3 color = ambient + Lo;

	outColor = vec4(color, 1.0f);
}