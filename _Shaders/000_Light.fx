
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

float4 VS_AllLight(MeshOutput input)
{
    Texture(Material.Diffuse, DiffuseMap, input.Uv);

    NormalMapping(input.Uv, input.Normal, input.Tangent);

    Texture(Material.Specular, SpecularMap, input.Uv);
   
    MaterialDesc output = MakeMaterial(), result = MakeMaterial();

    ComputeLight(output, input.Normal, input.wPosition);
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
