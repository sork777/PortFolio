
/////////////////////////////////////////////////////////////////////////////
// Directional Light
/////////////////////////////////////////////////////////////////////////////

struct DirShadow
{
    matrix ToShadowSpace;
    float4 ToCascadeOffsetX;
    float4 ToCascadeOffsetY;
    float4 ToCascadeScale;
};
cbuffer CB_DirShadow
{
    DirShadow DirCSM;
};
Texture2DArray CSMTexture;
//SamplerComparisonState PCFSampler;


/////////////////////////////////////////////////////////////////////////////
// Shadow
/////////////////////////////////////////////////////////////////////////////
struct ShadowCSM
{
    matrix ViewProjection[3];
    float2 ShadowMapSize;
    float ShadowBias;
};
cbuffer CB_CSM
{
    ShadowCSM cascadeShadow;
};

float4 ShadowGenVS(float4 Pos : POSITION) : SV_Position
{
    return Pos;
}

struct GS_CSM_Output
{
    float4 Position : SV_Position0;
    uint TargetIndex : SV_RenderTargetArrayIndex; //ArraySize의 번호
};

[maxvertexcount(9)]
void GS_PreRender(triangle MeshOutput input[3], inout TriangleStream<GS_CSM_Output> stream)
//void GS_PreRender(triangle float4 input[3] : SV_Position, inout TriangleStream<GS_CSM_Output> stream)
{
    //[unroll(8)]
    for (int i = 0; i < 3; i++)
    {
        GS_CSM_Output output;
        output.TargetIndex = i;
        for (int vertex = 0; vertex < 3; vertex++)
        {
            output.Position = mul(input[vertex].Position, cascadeShadow.ViewProjection[i]);
            //output.Position = mul(input[vertex], cascadeShadow.ViewProjection[i]);
            stream.Append(output);
        }
        stream.RestartStrip();
    }

}

/////////////////////////////////////////////////////////////////////////////
// Ambient Light
/////////////////////////////////////////////////////////////////////////////
float3 CalcAmbient(float3 normal, float3 color)
{
	// Convert from [-1, 1] to [0, 1]
    float up = normal.y * 0.5 + 0.5;

	// Calculate the ambient value
    float3 ambient = Material.Ambient + up * (Material.Diffuse - Material.Ambient);

	// Apply the ambient value to the color
    return ambient * color;
}


/////////////////////////////////////////////////////////////////////////////
// CSM Shadow
/////////////////////////////////////////////////////////////////////////////
float CascadedShadow(float3 position)
{
	// Transform the world position to shadow space
    float4 posShadowSpace = mul(float4(position, 1.0), DirCSM.ToShadowSpace);

	// Transform the shadow space position into each cascade position
    float4 posCascadeSpaceX = (DirCSM.ToCascadeOffsetX + posShadowSpace.xxxx) * DirCSM.ToCascadeScale;
    float4 posCascadeSpaceY = (DirCSM.ToCascadeOffsetY + posShadowSpace.yyyy) * DirCSM.ToCascadeScale;

	// Check which cascade we are in
    float4 inCascadeX = abs(posCascadeSpaceX) <= 1.0;
    float4 inCascadeY = abs(posCascadeSpaceY) <= 1.0;
    float4 inCascade = inCascadeX * inCascadeY;

	// Prepare a mask for the highest quality cascade the position is in
    float4 bestCascadeMask = inCascade;
    bestCascadeMask.yzw = (1.0 - bestCascadeMask.x) * bestCascadeMask.yzw;
    bestCascadeMask.zw = (1.0 - bestCascadeMask.y) * bestCascadeMask.zw;
    bestCascadeMask.w = (1.0 - bestCascadeMask.z) * bestCascadeMask.w;
    float bestCascade = dot(bestCascadeMask, float4(0.0, 1.0, 2.0, 3.0));

	// Pick the position in the selected cascade
    float3 UVD;
    UVD.x = dot(posCascadeSpaceX, bestCascadeMask);
    UVD.y = dot(posCascadeSpaceY, bestCascadeMask);
    UVD.z = posShadowSpace.z;

	// Convert to shadow map UV values
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;

    //가우시안 블러
    float shadow = 0;
    {
        float2 size = 1.0f / cascadeShadow.ShadowMapSize;
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

        //[unroll(9)]
        for (int i = 0; i < 25; i++)
        {
            uv = UVD.xy + offsets[i];
            totalweight += weight[i];
            
	        // Compute the hardware PCF value
            sum += CSMTexture.SampleCmpLevelZero(PCFSampler, float3(uv, bestCascade), UVD.z) * weight[i];
        }
        shadow = sum / totalweight;

    }
	
	// set the shadow to one (fully lit) for positions with no cascade coverage
    shadow = saturate(shadow + 1.0 - any(bestCascadeMask));
	
    return shadow;
}


/////////////////////////////////////////////////////////////////////////////
// Light Calculation
/////////////////////////////////////////////////////////////////////////////
float3 CalcDirectional(float3 position, DeferredMaterial material)
{
	// Phong diffuse
    float NDotL = dot(-GlobalLight.Direction, material.normal);
    float3 finalColor = GlobalLight.Specular.rgb * NDotL;
   
	// Blinn specular
    float3 ToEye = ViewPosition() - position;
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye - GlobalLight.Direction);
    float NDotH = saturate(dot(HalfWay, material.normal));
    float3 R = normalize(reflect(GlobalLight.Direction, material.normal)); /* 들어오는 방향 */
    float RdotE = saturate(dot(R, ToEye));

    finalColor += GlobalLight.Specular.rgb * pow(NDotH, material.specPow) * material.specIntensity;
    float shadowAtt = CascadedShadow(position);

    return finalColor * material.diffuseColor.rgb * shadowAtt;
}


/////////////////////////////////////////////////////////////////////////////
// Point Light
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// 변수들
/////////////////////////////////////////////////////////////////////////////

struct sPointLightDesc
{
    float4x4 LightProjection;
    float4 PointColor;
    float3 PointLightPos;
    float PointLightRangeRcp;
    float2 LightPerspectiveValues;
    float Specular;
};

cbuffer CB_PointLights
{
    sPointLightDesc cbPointLight;
};

static const float3 HemilDir[2] =
{
    float3(1.0, 1.0, 1.0),
	float3(-1.0, 1.0, -1.0)
};

matrix PL_CubeViewProj[6];
TextureCube PointShadowMapTexture;
uint bDrawPointShadow;

/////////////////////////////////////////////////////////////////////////////
// Shadow 
/////////////////////////////////////////////////////////////////////////////
struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    uint RTIndex : SV_RenderTargetArrayIndex;
};

[maxvertexcount(18)]
void PointShadowGenGS(triangle MeshOutput InPos[3] , inout TriangleStream<GS_OUTPUT> OutStream)
//void PointShadowGenGS(triangle float4 InPos[3] : SV_Position, inout TriangleStream<GS_OUTPUT> OutStream)
{
    for (int iFace = 0; iFace < 6; iFace++)
    {
        GS_OUTPUT output;

        output.RTIndex = iFace;

        for (int v = 0; v < 3; v++)
        {
            output.Pos = mul(InPos[v].Position, PL_CubeViewProj[iFace]);
            OutStream.Append(output);
        }
        OutStream.RestartStrip();
    }
}

float PointShadowPCF(float3 ToPixel)
{
    float3 ToPixelAbs = abs(ToPixel);
    float Z = max(ToPixelAbs.x, max(ToPixelAbs.y, ToPixelAbs.z));
    float Depth = (cbPointLight.LightPerspectiveValues.x * Z + cbPointLight.LightPerspectiveValues.y) / Z;
    
    return PointShadowMapTexture.SampleCmpLevelZero(PCFSampler, ToPixel, Depth);
}

/////////////////////////////////////////////////////////////////////////////
// Light Calculation
/////////////////////////////////////////////////////////////////////////////
float3 CalcPoint(float3 position, DeferredMaterial material)
{
    float3 ToLight = cbPointLight.PointLightPos - position;
    float3 ToEye = ViewPosition() - position;
    float DistToLight = length(ToLight);
 
   // Phong diffuse
    ToLight /= DistToLight; // Normalize
    
    float NDotL = saturate(dot(ToLight, material.normal));
    float3 finalColor = material.diffuseColor.rgb * NDotL;
 
   // Blinn specular
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(HalfWay, material.normal));
    //float3 R = normalize(reflect(-ToLight, material.normal));
    //float RdotE = saturate(dot(R, normalize(position)));

    finalColor += pow(NDotH, material.specPow.x) * material.specIntensity;

   // 감쇄
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbPointLight.PointLightRangeRcp);
    float Attn = (DistToLightNorm * DistToLightNorm);
    finalColor *= cbPointLight.PointColor.rgb * Attn * cbPointLight.Specular;
 
    float shadowAtt = 1.0f;
    
    
    if (bDrawPointShadow == 1)
    {
        shadowAtt = PointShadowPCF(position - cbPointLight.PointLightPos);
    }
    return finalColor * shadowAtt;
}

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 PointLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}
/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_CONSTANT_DATA_OUTPUT
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_CONSTANT_DATA_OUTPUT PointLightConstantHS()
{
    HS_CONSTANT_DATA_OUTPUT Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_OUTPUT
{
    //반구 방향?
    float4 HemiDir : POSITION0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("PointLightConstantHS")]
HS_OUTPUT PointLightHS(uint PatchID : SV_PrimitiveID)
{
    HS_OUTPUT Output;

    Output.HemiDir = float4(HemilDir[PatchID], 1);

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_OUTPUT
{
    float4 Position : SV_POSITION0;
    float2 cpPos : TEXCOORD0;
};

[domain("quad")]
DS_OUTPUT PointLightDS(HS_CONSTANT_DATA_OUTPUT input, float2 UV : SV_DomainLocation, const OutputPatch<HS_OUTPUT, 4> quad)
{
    // Uv값을 클립공간의 좌표값으로
    float2 posClipSpace = UV * 2.0 - 1.0;

	// 중심에서 가장 먼 절대값의 거리?
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// 클립공간의 최종값 추출
    float3 normDir = normalize(float3(posClipSpace.xy, (maxLen - 1.0)) * quad[0].HemiDir.xyz);
    float4 posLS = float4(normDir.xyz, 1.0);
	
	// 프로젝션공간으로 변환 후 uv 생성
    DS_OUTPUT Output;
    Output.Position = mul(posLS, cbPointLight.LightProjection);
	// 클립공간 위치 저장
    Output.cpPos = Output.Position.xy / Output.Position.w;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////
// Spot Light
/////////////////////////////////////////////////////////////////////////////

struct sSpotLightDesc
{
    float4 color;

    float3 Position;
    float RangeReverse;

    float3 Direction;
    float SpotCosOuterCone;

    float4x4 LightProjection;

    float SpotCosConeAttRange;

    float SinAngle;
    float CosAngle;
	float Specular;
	
    float OuterAngle;
    float InnerAngle;
};

cbuffer CB_SpotLights
{
    sSpotLightDesc cbSpotLight;
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 SpotLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_Const_SpotOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_Const_SpotOutput SpotLightConstantHS()
{
    HS_Const_SpotOutput Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_SPOT_OUTPUT
{
    float4 Position : POSITION0;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("SpotLightConstantHS")]
HS_SPOT_OUTPUT SpotLightHS()
{
    HS_SPOT_OUTPUT Output;

    Output.Position = float4(0.0, 0.0, 0.0,1.0);

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_SPOT_OUTPUT
{
    float4 Position : SV_POSITION0;
    float3 PositionXYW : TEXCOORD0;
};

#define CylinderPortion 0.2
#define ExpendAmount    (1.0 + CylinderPortion)

[domain("quad")]
DS_SPOT_OUTPUT SpotLightDS(HS_Const_SpotOutput input, float2 UV : SV_DomainLocation, const OutputPatch<HS_SPOT_OUTPUT, 4> quad)
{
	// Transform the UV's into clip-space
    float2 posClipSpace = UV.xy * float2(2.0, -2.0) + float2(-1.0, 1.0);

	// Find the vertex offsets based on the UV
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Force the cone vertices to the mesh edge
    float2 posClipSpaceNoCylAbs = saturate(posClipSpaceAbs * ExpendAmount);
    float maxLenNoCapsule = max(posClipSpaceNoCylAbs.x, posClipSpaceNoCylAbs.y);
    float2 posClipSpaceNoCyl = sign(posClipSpace.xy) * posClipSpaceNoCylAbs;

	// Convert the positions to half sphere with the cone vertices on the edge
    float3 halfSpherePos = normalize(float3(posClipSpaceNoCyl.xy, 1.0 - maxLenNoCapsule));

	// Scale the sphere to the size of the cones rounded base
    halfSpherePos = normalize(float3(halfSpherePos.xy * cbSpotLight.SinAngle, cbSpotLight.CosAngle));

	// Find the offsets for the cone vertices (0 for cone base)
    float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Offset the cone vertices to thier final position
    float4 posLS = float4(halfSpherePos.xy * (1.0 - cylinderOffsetZ), halfSpherePos.z - cylinderOffsetZ * cbSpotLight.CosAngle, 1.0);

	// Transform all the way to projected space and generate the UV coordinates
    DS_SPOT_OUTPUT Output;
    Output.Position = mul(posLS, cbSpotLight.LightProjection);
    Output.PositionXYW = Output.Position.xyw;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////////////////////////////////
Texture2D SpotShadowMapTexture;
matrix ToShadowMap;
uint bDrawSpotShadow;

float4 VS_Mesh_Spot(VertexMesh input) : SV_Position
{
    MeshOutput output;
    return mul(WorldPosition(input.Position), ToShadowMap);

}

float4 VS_Model_Spot(VertexModel input) : SV_Position
{
    MeshOutput output;
    SetModelWorld(World, input);
    return mul(WorldPosition(input.Position), ToShadowMap);

}

float4 VS_Animation_Spot(VertexModel input) : SV_Position
{
    MeshOutput output;
    SetAnimationWorld(World, input);
    return mul(WorldPosition(input.Position), ToShadowMap);

}
// Shadow PCF calculation helper function
float SpotShadowPCF(float3 position)
{
	// Transform the world position to shadow projected space
    float4 posShadowMap = mul(float4(position, 1.0), ToShadowMap);

	// Transform the position to shadow clip space
    float3 UVD = posShadowMap.xyz / posShadowMap.w;

	// Convert to shadow map UV values
    UVD.xy = 0.5 * UVD.xy + 0.5;
    UVD.y = 1.0 - UVD.y;

	// Compute the hardware PCF value
    return SpotShadowMapTexture.SampleCmpLevelZero(PCFSampler, UVD.xy, UVD.z);
}

float3 CalcSpot(float3 position, DeferredMaterial material)
{
    float3 ToLight = cbSpotLight.Position - position;
    float3 ToEye = ViewPosition() - position;
    float DistToLight = length(ToLight);

	// Phong diffuse
    ToLight /= DistToLight; // Normalize
    float NDotL = saturate(dot(ToLight, material.normal));
    float3 finalColor = material.diffuseColor.rgb * NDotL;

	// Blinn specular
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(HalfWay, material.normal));
    finalColor += pow(NDotH, material.specPow) * material.specIntensity;

	// Cone attenuation
    float cosAng = dot(cbSpotLight.Direction, ToLight);
    float conAtt = saturate((cosAng - cbSpotLight.SpotCosOuterCone) / cbSpotLight.SpotCosConeAttRange);
    conAtt *= conAtt;
   
    float shadowAtt=1.0f;
    if (bDrawSpotShadow == 1)
    {
        shadowAtt = SpotShadowPCF(position);
    }

	// Attenuation
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbSpotLight.RangeReverse);
    float Attn = DistToLightNorm * DistToLightNorm;
    finalColor *= cbSpotLight.color.rgb * Attn * conAtt * shadowAtt * cbSpotLight.Specular;
   
	// Return the fianl color
    return finalColor;
}

/////////////////////////////////////////////////////////////////////////////
// Capsule Light
/////////////////////////////////////////////////////////////////////////////

struct sCapsuleLightDesc
{
    float4 color;

    float3 Position;
    float RangeRcp;

    float3 Direction;
    float Length;

    float4x4 LightProjection;
    float HalfLen;
    float Range;
	float Specular;
};

cbuffer CB_CapsuleLights
{
    sCapsuleLightDesc cbCapsuleLight;
};

/////////////////////////////////////////////////////////////////////////////
// Vertex shader
/////////////////////////////////////////////////////////////////////////////
float4 CapsuleLightVS() : SV_Position
{
    return float4(0.0, 0.0, 0.0, 1.0);
}

/////////////////////////////////////////////////////////////////////////////
// Hull shader
/////////////////////////////////////////////////////////////////////////////
struct HS_ConstantCapsuleOutput
{
    float Edges[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};

HS_ConstantCapsuleOutput CapsuleLightConstantHS()
{
    HS_ConstantCapsuleOutput Output;
	
    float tessFactor = 18.0;
    Output.Edges[0] = Output.Edges[1] = Output.Edges[2] = Output.Edges[3] = tessFactor;
    Output.Inside[0] = Output.Inside[1] = tessFactor;

    return Output;
}

struct HS_CapsuleOutput
{
    float4 CapsuleDir : POSITION;
};

static const float4 CapsuelDir[2] =
{
    float4(1.0, 1.0, 1.0, 1.0),
	float4(-1.0, 1.0, -1.0, 1.0)
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("CapsuleLightConstantHS")]
HS_CapsuleOutput CapsuleLightHS(uint PatchID : SV_PrimitiveID)
{
    HS_CapsuleOutput Output;

    Output.CapsuleDir = CapsuelDir[PatchID];

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Domain Shader shader
/////////////////////////////////////////////////////////////////////////////
struct DS_CapsuleOutput
{
    float4 Position : SV_POSITION;
    float2 cpPos : TEXCOORD0;
};

#define CylinderPortion 0.2
#define SpherePortion   (1.0 - CylinderPortion)
#define ExpendAmount    (1.0 + CylinderPortion)

[domain("quad")]
DS_CapsuleOutput CapsuleLightDS(HS_ConstantCapsuleOutput input, float2 UV : SV_DomainLocation, const OutputPatch<HS_CapsuleOutput, 4> quad)
{
	// Transform the UV's into clip-space
    float2 posClipSpace = UV.xy * float2(2.0, -2.0) + float2(-1.0, 1.0);

	// Find the vertex offsets based on the UV
    float2 posClipSpaceAbs = abs(posClipSpace.xy);
    float maxLen = max(posClipSpaceAbs.x, posClipSpaceAbs.y);

	// Force the cone vertices to the mesh edge
    float2 posClipSpaceNoCylAbs = saturate(posClipSpaceAbs * ExpendAmount);
    float maxLenNoCapsule = max(posClipSpaceNoCylAbs.x, posClipSpaceNoCylAbs.y);
    float2 posClipSpaceNoCyl = sign(posClipSpace.xy) * posClipSpaceNoCylAbs;

	// Convert the positions to half sphere with the cone vertices on the edge
    float3 halfSpherePos = normalize(float3(posClipSpaceNoCyl.xy, 1.0 - maxLenNoCapsule));

	// Find the offsets for the cone vertices (0 for cone base)
    float cylinderOffsetZ = saturate((maxLen * ExpendAmount - 1.0) / CylinderPortion);

	// Apply the range
    halfSpherePos *= cbCapsuleLight.Range;

	// Offset the cone vertices to thier final position
    float4 posLS = float4(halfSpherePos.xy, halfSpherePos.z + cbCapsuleLight.HalfLen - cylinderOffsetZ * cbCapsuleLight.HalfLen, 1.0);

	// Move the vertex to the selected capsule side
    posLS *= quad[0].CapsuleDir;

	// Transform all the way to projected space and generate the UV coordinates
    DS_CapsuleOutput Output;
    Output.Position = mul(posLS, cbCapsuleLight.LightProjection);
    Output.cpPos = Output.Position.xy / Output.Position.w;

    return Output;
}

/////////////////////////////////////////////////////////////////////////////
// Pixel shader
/////////////////////////////////////////////////////////////////////////////
float3 CalcCapsule(float3 position, DeferredMaterial material)
{
    float3 ToEye = ViewPosition() - position;
   
   // Find the shortest distance between the pixel and capsules segment
    float3 ToCapsuleStart = position - cbCapsuleLight.Position;
    float DistOnLine = dot(ToCapsuleStart, cbCapsuleLight.Direction) / cbCapsuleLight.Length;
    DistOnLine = saturate(DistOnLine) * cbCapsuleLight.Length;
    float3 PointOnLine = cbCapsuleLight.Position + cbCapsuleLight.Direction * DistOnLine;
    float3 ToLight = PointOnLine - position;
    float DistToLight = length(ToLight);
   
   // Phong diffuse
    ToLight /= DistToLight; // Normalize
    float NDotL = saturate(dot(ToLight, material.normal));
    float3 finalColor = material.diffuseColor.rgb * NDotL;
   
   // Blinn specular
    ToEye = normalize(ToEye);
    float3 HalfWay = normalize(ToEye + ToLight);
    float NDotH = saturate(dot(HalfWay, material.normal));
    finalColor += pow(NDotH, material.specPow) * material.specIntensity;
   
   // Attenuation
    float DistToLightNorm = 1.0 - saturate(DistToLight * cbCapsuleLight.RangeRcp);
    float Attn = DistToLightNorm * DistToLightNorm;
    finalColor *= cbCapsuleLight.color.rgb * Attn * cbCapsuleLight.Specular;
   
    return finalColor;
}
