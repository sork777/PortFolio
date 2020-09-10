#include "000_Header.fx"
#include "000_Light.fx"
//#include "000_Model.fx"


struct VertexIntput
{
    float4 Position :Position0;
};

struct VertexOutput
{
    float4 Position : Position0;
};
VertexOutput VS(VertexIntput input)
{
    VertexOutput output;
   
    output.Position = input.Position;
   
    return output;
};

struct BillGeometryOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
};

bool FixedY;
float2 WindParm;
float2 BillScale;

[maxvertexcount(4)]
void GS(point VertexOutput input[1],inout TriangleStream<BillGeometryOutput>stream)
{
    float3 up = float3(0, 1, 0);
    float3 forward = ViewPosition() - input[0].Position.xyz;
    
    [flatten]
    if(FixedY == true)
        forward.y = 0.0f;
    forward = normalize(forward);
    
    
    float3 right = cross(forward, up);
    float2 size = BillScale * 0.5f;

    float3 position[4];
    position[0] = float3(input[0].Position.xyz - size.x * right - size.y * up); //¢×
    position[1] = float3(input[0].Position.xyz - size.x * right + size.y * up); //¢Ø
    position[2] = float3(input[0].Position.xyz + size.x * right - size.y * up); //¢Ù
    position[3] = float3(input[0].Position.xyz + size.x * right + size.y * up); //¢Ö

    float2 uvs[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };
    BillGeometryOutput output;

   //[roll(4)]
    for (int i = 0; i < 4;i++)
    {
        position[i] += float3(WindParm.x,0, WindParm.y) * (1 - uvs[i].y);
        output.Position = WorldPosition(float4(position[i], 1));
        output.Position = ViewProjection(output.Position);
        output.Uv = uvs[i];

        stream.Append(output);
    }
}

Texture2D BillTex;

float4 PS_Billboard(BillGeometryOutput input) : SV_Target0
{
    float4 diffuse = BillTex.Sample(LinearSampler, input.Uv);

    [flatten]
    if(diffuse.a<0.3f)
        discard;

    return diffuse;
}
/////////////////////////////////////////////////////////////////////////////////////////

struct BillDepthGeometryOutput
{
    float4 Position : SV_Position0;
    float4 sPosition : Position1;
    float2 Uv : Uv0;
};
[maxvertexcount(4)]
void DepthGS(point VertexOutput input[1], inout TriangleStream<BillDepthGeometryOutput> stream)
{
    float3 up = float3(0, 1, 0);
    float3 forward = ViewPosition() - input[0].Position.xyz;
    [flatten]
    if (FixedY == 1)
        forward.y = 0.0f;

    forward = normalize(forward);
    float3 right = cross(forward, up);
    float2 size = BillScale * 0.5f;

    float3 position[4];
    position[0] = float3(input[0].Position.xyz - size.x * right - size.y * up); //¢×
    position[1] = float3(input[0].Position.xyz - size.x * right + size.y * up); //¢Ø
    position[2] = float3(input[0].Position.xyz + size.x * right - size.y * up); //¢Ù
    position[3] = float3(input[0].Position.xyz + size.x * right + size.y * up); //¢Ö

    float2 uvs[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };
    BillDepthGeometryOutput output;

   //[roll(4)]
    for (int i = 0; i < 4; i++)
    {
        position[i] += float3(WindParm.x, 0, WindParm.y) * (1 - uvs[i].y);
        output.Position = WorldPosition(float4(position[i], 1));
        //output.Position = ViewProjection(output.Position);
        output.Position = mul(output.Position, ShadowView);
        output.Position = mul(output.Position, ShadowProjection);
        output.sPosition = output.Position;
        output.Uv = uvs[i];

        stream.Append(output);
    }
}

float4 PS_BillboardDepth(BillDepthGeometryOutput input) : SV_Target0
{
    float4 diffuse = BillTex.Sample(LinearSampler, input.Uv);
    float depth = input.sPosition.z / input.sPosition.w;
    //return diffuse;
    [flatten]
    if (diffuse.a < 0.3f)
        discard;
    return float4(depth, depth, depth, 1);
}


RasterizerState RS
{
    FillMode = Wireframe;
};
technique11 T0
{
    //P_VP(P0, VS_Mesh, PS)
    //P_VP(P1, VS_Model, PS)
    P_VGP(P0, VS, GS, PS_Billboard)
    P_RS_VGP(P1,RS, VS, GS, PS_Billboard)

}
technique11 TShadow
{
    P_VGP(P0, VS, DepthGS, PS_BillboardDepth)
}