#include "D:\GitHubPJ\PF\PortFolio\_Shaders/LightPBR/PBRHeader.fx"

void CalcRadiance(float3 wPosition, float3 viewDir, float3 NormalVec, float3 albedo, float roughness, float metallic, float3 lightPos, float3 lightCol, float3 F0, out float3 rad)
{
    //static const floatPI = 3.14159265359;

	//calculate light radiance
    float3 lightDir = normalize(lightPos - wPosition);
    float3 halfwayVec = normalize(viewDir + lightDir);
    float distance = length(lightPos - wPosition);
    float attenuation = 1.0f / (distance * distance);
    float3 radiance = lightCol * attenuation;

	//Cook-Torrance BRDF
    float D = NormalDistributionGGXTR(NormalVec, halfwayVec, roughness);
    float G = GeometrySmith(NormalVec, viewDir, lightDir, roughness);
    float3 F = FresnelSchlick(max(dot(halfwayVec, viewDir), 0.0f), F0);

    float3 kS = F;
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0 - metallic;

    float3 nom = D * G * F;
    float denom = 4 * max(dot(NormalVec, viewDir), 0.0f) * max(dot(NormalVec, lightDir), 0.0) + 0.001f; // 0.001f just in case product is 0
    float3 specular = nom / denom;

	//Add to outgoing radiance Lo
    float NdotL = max(dot(NormalVec, lightDir), 0.0f);
    rad = (((kD * albedo / PI) + specular) * radiance * NdotL);
}

float4 PBRmain(MeshOutput input)
{
	//static const float PI = 3.14159265359;
	/*float3 albedo;
	float metallic;
	float roughness;*/
	//Albedo
    float3 albedo = AlbedoMap.Sample(BasicSampler, input.Uv).rgb;
    albedo *= MaterialPBR.Albedo;
    albedo = pow(albedo, 2.2f);

    NormalMapping(input.Uv, input.Normal, input.Tangent, BasicSampler);
	
    float3 NormalVec = normalize(input.Normal);
	
	//Metallic
    float metallic = MetalicMap.Sample(BasicSampler, input.Uv).r;

	//Rough
    float rough = LoughnessMap.Sample(BasicSampler, input.Uv).r;
	
    float3 viewDir = normalize(ViewPosition() - input.wPosition);
	
    float3 R = reflect(-viewDir, NormalVec);

    float3 F0 = MaterialPBR.F0;
    F0 = lerp(F0, albedo, metallic);

    float3 rad = float3(0.0f, 0.0f, 0.0f);
	//reflectance equation
    float3 Lo = float3(0.0f, 0.0f, 0.0f);
    
    float3 lightpos = GlobalLight.Postition-GlobalLight.Direction*100;
    float3 lightCol = GlobalLight.Ambient;
    // ºû°úÀÇ ¹Ý»ç?
    CalcRadiance(input.wPosition, viewDir, NormalVec, albedo, rough, metallic, lightpos, lightCol, F0, rad);
    Lo += rad;
    
    float3 kS = FresnelSchlickRoughness(max(dot(NormalVec, viewDir), 0.0f), F0, rough);
    float3 kD = float3(1.0f, 1.0f, 1.0f) - kS;
    kD *= 1.0 - metallic;

    float3 irradiance = CalcSkyIrradiance(NormalVec);
    float3 diffuse = albedo * irradiance;

    const float MAX_REF_LOD = 4.0f;
    float3 prefilteredColor = SkyCubeMap.SampleLevel(BasicSampler, R, rough * MAX_REF_LOD).rgb;
    float2 brdf = BRDFLUT.Sample(BasicSampler, float2(max(dot(NormalVec, viewDir), 0.0f), rough)).rg;
    float3 specular = prefilteredColor * (kS * brdf.x + brdf.y);

    float3 ambient = (kD * diffuse + specular); // * ao;
    float3 color = ambient+Lo;
    //float3 color = ambient;
    float NdotE = dot(NormalVec, viewDir);
    float emissive = smoothstep(1.0f - MaterialPBR.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
    float3 emiss = MaterialPBR.Emissive.rgb * emissive;
    
    // color = color / (color + float3(1.0f, 1.0f, 1.0f));
    color = pow(color, float3(1.0f / 2.2f, 1.0f / 2.2f, 1.0f / 2.2f));
    color += emiss;
    return float4(color, 1.0f);
}