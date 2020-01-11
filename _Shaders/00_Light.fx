struct LightDesc
{
    float4 Ambient;
    float4 Specular;
    float3 Direction;
    float Padding;
    float3 Position;
};

struct MaterialDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;
};

cbuffer CB_Material
{
    MaterialDesc Material;
};

cbuffer CB_Light
{
    LightDesc GlobalLight;
};

float3 MaterialToColor(MaterialDesc result)
{
    return (result.Ambient + result.Diffuse + result.Specular + result.Emissive).rgb;
}

void AddMaterial(inout MaterialDesc result, MaterialDesc val)
{
    result.Ambient += val.Ambient;
    result.Diffuse += val.Diffuse;
    result.Specular += val.Specular;
    result.Emissive += val.Emissive;
}

MaterialDesc MakeMaterial()
{
    MaterialDesc output;
    output.Ambient = float4(0, 0, 0, 0);
    output.Diffuse = float4(0, 0, 0, 0);
    output.Specular = float4(0, 0, 0, 0);
    output.Emissive = float4(0, 0, 0, 0);

    return output;
}

///////////////////////////////////////////////////////////////////////////////

void Texture(inout float4 color, Texture2D t, float2 uv, SamplerState samp)
{
    float4 sampling = t.Sample(samp, uv);
    
    color = color * sampling;
}

void Texture(inout float4 color, Texture2D t, float2 uv)
{
    Texture(color, t, uv, LinearSampler);
}

///////////////////////////////////////////////////////////////////////////////

void ComputeLight(out MaterialDesc output, float3 normal, float3 wPosition)
{
    output.Ambient = 0;
    output.Diffuse = 0;
    output.Specular = 0;
    
    float3 direction = -GlobalLight.Direction;
    float NdotL = dot(direction, normalize(normal));
    
    output.Ambient = GlobalLight.Ambient * Material.Ambient;
    
    [flatten]
    if(NdotL > 0.0f)
    {
        output.Diffuse = NdotL * Material.Diffuse;
        
        
        float3 E = normalize(ViewPosition() - wPosition);
        
        [flatten]
        if(any(Material.Specular.rgb))
        {
            float3 R = normalize(reflect(-direction, normal));    
            float RdotE = saturate(dot(R, E));

            float specular = pow(RdotE, Material.Specular.a);
            output.Specular = specular * Material.Specular * GlobalLight.Specular;
        }
        
        [flatten]
        if(any(Material.Emissive.rgb))
        {
            float NdotE = dot(normalize(normal), E);
            float emissive = smoothstep(1.0f - Material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
            
            output.Emissive = Material.Emissive * emissive;
        }
    }
}

void NormalMapping(float2 uv, float3 normal, float3 tangent, SamplerState samp)
{
    float4 map = NormalMap.Sample(samp, uv);
    
    [flatten]
    if (any(map) == false)
        return;

    
    //≈∫¡®∆Æ ∞¯∞£
    float3 N = normalize(normal); //Z
    float3 T = normalize(tangent - dot(tangent, N) * N); //X
    float3 B = cross(N, T); //Y
    float3x3 TBN = float3x3(T, B, N);
    
    //¿ÃπÃ¡ˆ «»ºø∑Œ∫Œ≈Õ ≥Î∏÷∫§≈Õ πÊ«‚ ∏∏µÈ±‚
    float3 coord = map.rgb * 2.0f - 1.0f;
    
    //≈∫¡®∆Æ ∞¯∞£¿∏∑Œ ∫Ø»Ø
    coord = mul(coord, TBN);
    
    Material.Diffuse *= saturate(dot(coord, -GlobalLight.Direction));
}

void NormalMapping(float2 uv, float3 normal, float3 tangent)
{
    NormalMapping(uv, normal, tangent, LinearSampler);
}

///////////////////////////////////////////////////////////////////////////////

struct PointLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;

    float3 Position;
    float Range;

    float Intensity;
    float3 Padding;
};

#define MAX_POINT_LIGHT 32
cbuffer CB_PointLights
{
    uint PointLightCount;
    float3 CB_PointLights_Padding;

    PointLightDesc PointLights[MAX_POINT_LIGHT];
};

void ComputePointLight(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output = MakeMaterial();
    MaterialDesc result = MakeMaterial();
    
    //[unroll(MAX_POINT_LIGHT)]
    for (uint i = 0; i < PointLightCount; i++)
    {
        float3 light = PointLights[i].Position - wPosition;
        float dist = length(light);
        
        [flatten]
        if (dist > PointLights[i].Range)
            continue;
        
        light /= dist;
        
        result.Ambient = Material.Ambient * PointLights[i].Ambient;
        float NdotL = dot(light, normalize(normal));
        
        [flatten]
        if (NdotL > 0.0f)
        {
            float3 E = normalize(ViewPosition() - wPosition);
        
            float3 R = normalize(reflect(-light, normal));
            float RdotE = saturate(dot(R, E));
            float specular = pow(RdotE, Material.Specular.a);
            
            float NdotE = dot(normalize(normal), E);
            float emissive = smoothstep(1.0f - Material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
            
            result.Diffuse = NdotL * Material.Diffuse * PointLights[i].Diffuse;
            result.Specular = specular * Material.Specular * PointLights[i].Specular;
            result.Emissive = emissive * Material.Emissive * PointLights[i].Emissive;
        }
     
        
        float temp = 1.0f / saturate(dist / PointLights[i].Range);
        float att = temp * temp * (1.0f / max(1 - PointLights[i].Intensity, 1e-8f));
        
        output.Ambient += result.Ambient * temp;
        output.Diffuse += result.Diffuse * att;
        output.Specular += result.Specular * att;
        output.Emissive += result.Emissive * att;
    }
}

///////////////////////////////////////////////////////////////////////////////

struct SpotLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;
    float4 Emissive;

    float3 Position;
    float Range;

    float3 Direction;
    float Angle;

    float Intensity;
    float3 Padding;
};

#define MAX_SPOT_LIGHT 32
cbuffer CB_SpotLights
{
    uint SpotLightCount;
    float3 CB_SpotLights_Padding;

    SpotLightDesc SpotLights[MAX_SPOT_LIGHT];
};

void ComputeSpotLight(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output = MakeMaterial();
    MaterialDesc result = MakeMaterial();
    
    //[unroll(MAX_SPOT_LIGHT)]
    for (uint i = 0; i < SpotLightCount; i++)
    {
        float3 light = SpotLights[i].Position - wPosition;
        float dist = length(light);
        
        [flatten]
        if (dist > SpotLights[i].Range)
            continue;
        
        light /= dist;
        
        result.Ambient = Material.Ambient * SpotLights[i].Ambient;
        float NdotL = dot(light, normalize(normal));
        
        [flatten]
        if (NdotL > 0.0f)
        {
            float3 E = normalize(ViewPosition() - wPosition);
        
            float3 R = normalize(reflect(-light, normal));
            float RdotE = saturate(dot(R, E));
            float specular = pow(RdotE, Material.Specular.a);
            
            float NdotE = dot(normalize(normal), E);
            float emissive = smoothstep(1.0f - Material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
            
            result.Diffuse = NdotL * Material.Diffuse * SpotLights[i].Diffuse;
            result.Specular = specular * Material.Specular * SpotLights[i].Specular;
            result.Emissive = emissive * Material.Emissive * SpotLights[i].Emissive;
        }
     
        
        float temp = pow(saturate(dot(-light, SpotLights[i].Direction)), SpotLights[i].Angle);
        float att = temp * (1.0f / max(1 - SpotLights[i].Intensity, 1e-8f));
        
        output.Ambient += result.Ambient * temp;
        output.Diffuse += result.Diffuse * att;
        output.Specular += result.Specular * att;
        output.Emissive += result.Emissive * att;
    }
}