
float ConvertZToLinearDepth(float depth)
{
    float linearDepth = PerspectiveValues.z / (depth + PerspectiveValues.w);
    return linearDepth;
}

SURFACE_DATA UnpackGBuffer(float2 uv)
{
    SURFACE_DATA Out;

    float depth = DepthTexture.Sample(PointSampler, uv.xy).x;
    Out.LinearDepth = ConvertZToLinearDepth(depth);
    float4 baseColorSpecInt = ColorSpecIntTexture.Sample(PointSampler, uv.xy);
    float4 NormalValue = NormalTexture.Sample(PointSampler, uv.xy);
    Out.Color = baseColorSpecInt.xyz;
    Out.SpecIntensity = baseColorSpecInt.w;
    Out.Normal = NormalValue.xyz;
    Out.Normal = normalize(Out.Normal * 2.0 - 1.0);
    Out.SpecPow = SpecPowTexture.Sample(PointSampler, uv.xy).xyz;

    return Out;
}

SURFACE_DATA UnpackGBuffer_Loc(int2 location)
{
    SURFACE_DATA Out;
    int3 location3 = int3(location, 0);

    float depth = DepthTexture.Load(location3).x;
    Out.LinearDepth = ConvertZToLinearDepth(depth);
    float4 baseColorSpecInt = ColorSpecIntTexture.Load(location3);
    Out.Color = baseColorSpecInt.xyz;
    Out.SpecIntensity = baseColorSpecInt.w;
    Out.Normal = NormalTexture.Load(location3).xyz;
    Out.Normal = normalize(Out.Normal * 2.0 - 1.0);
    Out.SpecPow = SpecPowTexture.Load(location3).xyz;

    return Out;
}

/////////////////////////////////////////////////////////////////////////////

void MaterialFromGBuffer(SURFACE_DATA gbd, inout DeferredMaterial mat)
{
    mat.normal = gbd.Normal;
    mat.diffuseColor.xyz = gbd.Color;
    mat.diffuseColor.w = 1.0; // Fully opaque
    mat.specPow = 10+250 * gbd.SpecPow;
    mat.specIntensity = gbd.SpecIntensity;
}

/////////////////////////////////////////////////////////////////////////////
float3 CalNormal(float2 uv, float3 normal, float3 tangent, SamplerState sample)
{
    float4 map = NormalMap.Sample(sample, uv);
    
    [flatten]
    if (any(map) == false)
        return normal;

    //탄젠트 공간
    float3 N = normalize(normal); //Z
    float3 T = normalize(tangent - dot(tangent, N) * N); //X
    float3 B = cross(N, T); //Y
    float3x3 TBN = float3x3(T, B, N);

    //이미지로 부터 노멀벡터 가져오기
    float3 coord = map.rgb * 2.0f - 1.0f;

    return mul(coord, TBN);
}

float3 CalNormal(float2 uv, float3 normal, float3 tangent)
{
    return CalNormal(uv, normal, tangent, LinearSampler);
}
