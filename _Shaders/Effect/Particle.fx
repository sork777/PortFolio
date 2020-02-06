#include "../000_Header.fx"
#include "../000_Light.fx"

#define MAX_INSTANCE 4096

Texture2D ParticleTex;
/* Instancing ¿ë */
struct ParticleDesc
{
    matrix Global;
    float4 color;
};
cbuffer CB_Particle
{
    ParticleDesc Particle[MAX_INSTANCE];
};

struct VertexIntput
{
    float4 Position : Position0;
    float2 Scale : Scale0;

    uint InstID : SV_InstanceID;
};


struct VertexOutput
{
    float4 Position : Position0;
    float2 Scale : Scale0;
    
    uint ID : Id0;
};
VertexOutput VS(VertexIntput input)
{
    VertexOutput output;
   
	output.Position = input.Position;
	output.Scale = input.Scale;

    output.ID = input.InstID;
   
    return output;
};

struct GSOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
       
    uint ID : Id0;
};

[maxvertexcount(4)]
void GS(point VertexOutput input[1], inout TriangleStream<GSOutput> stream)
{
    float3 up = float3(0, 1, 0);
    float3 forward = ViewPosition() - input[0].Position.xyz;

    forward = normalize(forward);
    float3 right = cross(forward, up);
    float2 size = input[0].Scale * 0.5f;

    float3 position[4];
    position[0] = float3(input[0].Position.xyz - size.x * right - size.y * up);
    position[1] = float3(input[0].Position.xyz - size.x * right + size.y * up);
    position[2] = float3(input[0].Position.xyz + size.x * right - size.y * up);
    position[3] = float3(input[0].Position.xyz + size.x * right + size.y * up);

    float2 uvs[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };
    GSOutput output;

    matrix global = Particle[input[0].ID].Global;
   //[roll(4)]
    for (int i = 0; i < 4; i++)
    {
        output.Position = mul(float4(position[i], 1), global);
        output.Position = WorldPosition(output.Position);
        output.Position = ViewProjection(output.Position);
        output.Uv = uvs[i];
        output.ID = input[0].ID;
        stream.Append(output);
    }
}

float4 PS(GSOutput input) : SV_Target0
{
    float4 diffuse = ParticleTex.Sample(LinearSampler, input.Uv);
    float4 color = Particle[input.ID].color;

    return (diffuse*color);
}


RasterizerState RS
{
    FillMode = Wireframe;
};

technique11 T0
{
    P_VGP(P0, VS, GS, PS)
}