#define MAX_POINT_LIGHT 32
#define MAX_SPOT_LIGHT 32
#define MAX_CAPSULE_LIGHT 32

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

struct LightDesc
{
    float4 Ambient;
    float4 Specular;
    float3 Direction;
    float Padding;
    float3 Postition;//구조체에서 마지막은 패딩 안해줘도 됨?
};

cbuffer CB_Light
{
    LightDesc GlobalLight;
};

MaterialDesc MakeMaterial()
{
    MaterialDesc output;
    output.Ambient = float4(0, 0, 0, 0);
    output.Diffuse = float4(0, 0, 0, 0);
    output.Specular = float4(0, 0, 0, 0);
    output.Emissive = float4(0, 0, 0, 0);

    return output;
}

void AddMaterial(inout MaterialDesc result, MaterialDesc val)
{
    result.Ambient += val.Ambient;
    result.Diffuse += val.Diffuse;
    result.Specular += val.Specular;
    result.Emissive += val.Emissive;
}

float3 MaterialToColor(MaterialDesc result)
{
    return (result.Ambient + result.Diffuse + result.Specular + result.Emissive).rgb;
}

////////////////////////////////////////////////////////////////////////////////

void ComputeLight(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output.Ambient = 0;
    output.Diffuse = 0;
    output.Specular = 0;
    //output.Emissive = 0;
    
    float3 direction = -GlobalLight.Direction;  /* 나가는 방향 */

    float NdotL = dot(direction, normalize(normal));

                                    /* 같은 모델이어도 명암이 달라질때 바뀜 */
    output.Ambient = GlobalLight.Ambient * Material.Ambient;


    [flatten]
    if(NdotL>0.0f)
    {
        output.Diffuse = NdotL * Material.Diffuse;
        float3 E = normalize(ViewPosition() - wPosition);
        
        /* any 전부 0이면 false 아니면 true */        
        [flatten]
        if (any(Material.Specular.rgb))
        {
            float3 R = normalize(reflect(-direction, normal));
            float RdotE = saturate(dot(R, E));

            float specular = pow(RdotE, Material.Specular.a);
            output.Specular = specular * Material.Specular * GlobalLight.Specular;
        }
        
        [flatten]
        if (any(Material.Emissive.rgb))
        {
            float NdotE = dot(normalize(normal), E);
            float emissive = smoothstep(1.0f - Material.Emissive.a, 1.0f, 1.0f - saturate(NdotE));
            
            output.Emissive = Material.Emissive * emissive;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////

uint Selected;
/* 디퓨즈에 넣어 줄거라 리턴이 없다.*/
void NormalMapping(float2 uv, inout float3 normal, float3 tangent, SamplerState sample)
{
    float4 map = NormalMap.Sample(sample, uv);
    
    [flatten]
    if (any(map) == false)
        return;

    //탄젠트 공간
    float3 N = normalize(normal);   //Z
    //float3 T = normalize(tangent);  //이렇게 하면 안됨 정규 직교 해야함
    float3 T = normalize(tangent - dot(tangent, N) * N);    //X
    float3 B = cross(N, T);         //Y
    float3x3 TBN = float3x3(T, B, N);

    //이미지로 부터 노멀벡터 가져오기
    float3 coord = map.rgb * 2.0f -1.0f;

    normal = mul(coord, TBN);
    //float NdotL = saturate(dot(coord, -GlobalLight.Direction));
    //Material.Diffuse *= NdotL; //float4(coord * 0.5f + 0.5f, 1);
}

void NormalMapping(float2 uv, float3 normal, float3 tangent)
{
    NormalMapping(uv, normal, tangent, LinearSampler);
}

////////////////////////////////////////////////////////////////////////////////

struct PointLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;
  
    float Intensity;
    float3 Padding;
};

cbuffer CB_PointLights
{
    uint PointLightCount;
    float3 CB_PointLights_Padding;

    PointLightDesc PointLights[MAX_POINT_LIGHT];
};


void ComputePointLights(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output = MakeMaterial();
    
    MaterialDesc result = MakeMaterial();

    //[unroll(MAX_POINT_LIGHT)]
    for (int i = 0; i < PointLightCount; i++)
    {
        float3 light = PointLights[i].Position - wPosition;    //정점에서 포인트를 바라보는 방향
        float dist = length(light);

        [flatten]
        if(dist > PointLights[i].Range)
            continue;

        light /= dist; //정규화

        result.Ambient = Material.Ambient * PointLights[i].Ambient;
        
        float NdotL = dot(light, normalize(normal));

        [flatten]
        if(NdotL>0.0f)     
        {
            wPosition = ViewPosition() - wPosition;

            float3 R = normalize(reflect(-light, normal));
            float RdotE = saturate(dot(R, normalize(wPosition)));
            float shininess = pow(RdotE, Material.Specular.a);
            result.Diffuse = NdotL * Material.Diffuse * PointLights[i].Diffuse;

            result.Specular = shininess * Material.Specular * PointLights[i].Specular;

        }
      
        float temp = 1.0f - saturate(dist / PointLights[i].Range);
        float att;
        [flatten]
        if (PointLights[i].Intensity <= 0.0f)
        {
            att = 0.0f;
            result.Ambient = 0;
        }
        else
            att = temp * temp * (1.0f / PointLights[i].Intensity);
  
        output.Ambient += result.Ambient;
        output.Diffuse += result.Diffuse * att;
        output.Specular += result.Specular * att;

    }

}


////////////////////////////////////////////////////////////////////////////////

struct SpotLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;
  
    float3 Direction;
    float Angle;

    float Intensity;
    float3 Padding;
};

cbuffer CB_SpotLights
{
    uint SpotLightCount;
    float3 CB_SpotLights_Padding;

    SpotLightDesc SpotLights[MAX_SPOT_LIGHT];
};


void ComputeSpotLights(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output = MakeMaterial();
    
    MaterialDesc result = MakeMaterial();

    //[unroll(MAX_SPOT_LIGHT)]
    for (int i = 0; i < SpotLightCount; i++)
    {
        float3 start = wPosition - SpotLights[i].Position;
        float distFromStart = dot(start, SpotLights[i].Direction) / SpotLights[i].Range;
        distFromStart = saturate(distFromStart) * SpotLights[i].Range;
        /* 원뿔의 원의 중심 */
        float3 Center = SpotLights[i].Position +SpotLights[i].Direction * distFromStart;
        
        float3 light = SpotLights[i].Position - wPosition; //정점에서 포인트를 바라보는 방향
        float3 PointToCenter = Center - wPosition;                //정점에서 단면원의 중심을 바라보는 방향
        float dist = length(light);
        float dist2 = length(PointToCenter);            //spotlight에서 단면 원의 중심까지의 거리
        float len = length(distFromStart);  
        float range = distFromStart * tan(radians(SpotLights[i].Angle));
        
        [flatten]
        if (dist2 > range)
            continue;

        light /= dist; //정규화

        result.Ambient = Material.Ambient * SpotLights[i].Ambient;
        
        float NdotL = dot(light, normalize(normal));

        [flatten]
        if (NdotL > 0.0f)
        {
            wPosition = ViewPosition() - wPosition;

            float3 R = normalize(reflect(-light, normal));
            float RdotE = saturate(dot(R, normalize(wPosition)));
            float shininess = pow(RdotE, Material.Specular.a);
            result.Diffuse = NdotL * Material.Diffuse * SpotLights[i].Diffuse;

            result.Specular = shininess * Material.Specular * SpotLights[i].Specular;

        }
        //방향에 맞을때만, angle에 따라 퍼지니까.
        float spot = (pow(dot(-light, SpotLights[i].Direction), SpotLights[i].Angle));
        spot *= (1 - dist2 / range);
        spot *= SpotLights[i].Range/dist;
        spot = saturate(spot);
        float att;
        [flatten]
        if (SpotLights[i].Intensity <= 0.0f)
        {
            att = 0.0f;
            result.Ambient = 0;
        }
        else
            att = spot * (1.0f / SpotLights[i].Intensity);

        output.Ambient += result.Ambient * spot;
        output.Diffuse += result.Diffuse * att;
        output.Specular += result.Specular * att;

    }

}


////////////////////////////////////////////////////////////////////////////////

struct CapsuleLightDesc
{
    float4 Ambient;
    float4 Diffuse;
    float4 Specular;

    float3 Position;
    float Range;
  
    float3 Direction;
    float Length;

    float Intensity;
    float3 Padding;
};

cbuffer CB_CapsuleLights
{
    uint CapsuleLightCount;
    float3 CB_CapsuleLights_Padding;

    CapsuleLightDesc CapsuleLights[MAX_CAPSULE_LIGHT];
};


void ComputeCapsuleLights(inout MaterialDesc output, float3 normal, float3 wPosition)
{
    output = MakeMaterial();
    
    MaterialDesc result = MakeMaterial();

    //[unroll(MAX_CAPSULE_LIGHT)]
    for (int i = 0; i < CapsuleLightCount; i++)
    {
        //light 시작점
        float3 start = wPosition - CapsuleLights[i].Position;
        float distOnLine = dot(start, CapsuleLights[i].Direction) / CapsuleLights[i].Length;
        distOnLine = saturate(distOnLine) * CapsuleLights[i].Length;

        float3 pointOnLine = CapsuleLights[i].Position + CapsuleLights[i].Direction * distOnLine;
        float3 light = pointOnLine - wPosition;

        float dist = length(light);
        [flatten]
        if (dist > CapsuleLights[i].Range)
            continue;

        light /= dist; //정규화

        
        result.Ambient = Material.Ambient * CapsuleLights[i].Ambient;
        
        float NdotL = dot(light, normalize(normal));

        [flatten]
        if (NdotL > 0.0f)
        {
            wPosition = ViewPosition() - wPosition;

            float3 R = normalize(reflect(-light, normal));
            float RdotE = saturate(dot(R, normalize(wPosition)));
            float shininess = pow(RdotE, Material.Specular.a);
            result.Diffuse = NdotL * Material.Diffuse * CapsuleLights[i].Diffuse;

            result.Specular = shininess * Material.Specular * CapsuleLights[i].Specular;

        }
        
        float temp = 1.0f - saturate(dist / CapsuleLights[i].Range);
        float att;
        [flatten]
        if (CapsuleLights[i].Intensity <= 0.0f)
        {
            att = 0.0f;
            result.Ambient = 0;
        }
        else
            att = temp * temp * (1.0f / CapsuleLights[i].Intensity);

        output.Ambient += result.Ambient;
        output.Diffuse += result.Diffuse * att;
        output.Specular += result.Specular * att;

    }

}


////////////////////////////////////////////////////////////////////////////////

float4 VS_AllLight(MeshOutput input)
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);

    NormalMapping(input.Uv, input.Normal, input.Tangent);

    Texture(Material.Specular, SpecularMap, input.Uv);
   
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputePointLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputeSpotLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);

    ComputeCapsuleLights(output, input.Normal, input.wPosition);
    AddMaterial(result, output);
    
    return float4(MaterialToColor(result), 1);
}

float4 VS_AllLight(GeometryOutput input)
{
    MeshOutput output;

    output.Position = input.Position;
    output.wvpPosition = input.wvpPosition;
    output.oPosition = input.oPosition;
    output.wPosition = input.wPosition;
    output.sPosition = input.sPosition;
    output.Uv = input.Uv;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;

    return VS_AllLight(output);
}

float4 VS_Shadow(MeshOutput input, float4 color)
{
    input.sPosition.xyz /= input.sPosition.w;

    [flatten]
    if (input.sPosition.x < -1.0f || input.sPosition.x > 1.0f ||
        input.sPosition.y < -1.0f || input.sPosition.y > 1.0f ||
        input.sPosition.z < 0.0f || input.sPosition.z > 1.0f)
        return color;

    input.sPosition.x = input.sPosition.x * 0.5f + 0.5f;
    input.sPosition.y = -input.sPosition.y * 0.5f + 0.5f;
    input.sPosition.z -= ShadowBias;
    //return ShadowMap.Sample(LinearSampler, input.sPosition.xy);

    float depth = 0;
    float factor = 0;
   
    {
        float2 size = 1.0f / ShadowMapSize;
        float2 offsets[] =
        {
            float2(+2 * size.x, -2 * size.y), float2(+size.x, -2 * size.y), float2(0.0f, -2 * size.y), float2(-size.x, -2 * size.y), float2(-2 * size.x, -2 * size.y),
            float2(+2 * size.x, -size.y), float2(+size.x, -size.y), float2(0.0f, -size.y), float2(-size.x, -size.y), float2(-2 * size.x, -size.y),
            float2(+2 * size.x, 0.0f), float2(+size.x, 0.0f), float2(0.0f, 0.0f), float2(-size.x, 0.0f), float2(-2 * size.x, 0.0f),
            float2(+2 * size.x, +size.y), float2(+size.x, +size.y), float2(0.0f, +size.y), float2(-size.x, +size.y), float2(-2 * size.x, +size.y),
            float2(+2 * size.x, +2 * size.y), float2(+size.x, +2 * size.y), float2(0.0f, +2 * size.y), float2(-size.x, +2 * size.y), float2(-2 * size.x, +2 * size.y),
        };
        float weight[] =
        {
            1, 1, 2, 1, 1,
            1, 2, 4, 2, 1,
            2, 4, 8, 4, 2,
            1, 2, 4, 2, 1,
            1, 1, 2, 1, 1,
        };

        float sum = 0.0f;
        float totalweight = 0.0f;
        float2 uv = 0.0f;
        
        depth = input.sPosition.z;

        //[unroll(9)]
        for (int i = 0; i < 25; i++)
        {
            uv = input.sPosition.xy + offsets[i];
            totalweight += weight[i];
            sum += ShadowMap.SampleCmpLevelZero(ShadowSampler, uv, depth).r * weight[i];
        }

        factor = sum / totalweight;
    }
    factor = saturate(factor + depth);

    return float4(color.rgb * factor, 1);
}

float4 VS_Shadow(GeometryOutput input, float4 color)
{
    MeshOutput output;

    output.Position = input.Position;
    output.wvpPosition = input.wvpPosition;
    output.oPosition = input.oPosition;
    output.wPosition = input.wPosition;
    output.sPosition = input.sPosition;
    output.Uv = input.Uv;
    output.Normal = input.Normal;
    output.Tangent = input.Tangent;

    return VS_Shadow(output, color);
}
