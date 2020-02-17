#include "../000_Header.fx"
#include "../000_Light.fx"


Texture2D ParticleTex;

struct Particle
{
    matrix Global;
    float4 color;    
};


struct VertexIntput
{
    float4 Position : Position0;
    float2 Scale : Scale0;

    Particle particle : inst0;
};


struct VertexOutput
{
    float4 Position : Position0;
    float2 Scale : Scale0;
    
    matrix Global : Transform0;
    float4 color : Color0;
};

VertexOutput VS(VertexIntput input)
{
    VertexOutput output;
   
	output.Position = input.Position;
	output.Scale = input.Scale;

    output.Global = input.particle.Global;
    output.color = input.particle.color;
   
    return output;
};

struct GSOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;
       
    float4 color : Color0;
};

[maxvertexcount(4)]
void GS(point VertexOutput input[1], inout TriangleStream<GSOutput> stream)
{
    float3 up = float3(0, 1, 0);
    float3 forward = ViewPosition() - input[0].Position.xyz;

    forward = normalize(forward);
    float3 right = normalize(cross(forward, up));
    float dir = dot(up, forward);
    
    [flatten]
    if (length(right) <= 0.01f)
    {
        [branch]
        if (dir > 0)
        {
            right = float3(1, 0, 0);
        }
        else
            right = float3(-1, 0, 0);
    }
    up = (cross(right, forward));
    float3 x = normalize(right);
    float3 y = normalize(up); 
    float2 size = input[0].Scale * 0.5f;

    float3 position[4];
    position[0] = float3(input[0].Position.xyz - size.x * x - size.y * y);
    position[1] = float3(input[0].Position.xyz - size.x * x + size.y * y);
    position[2] = float3(input[0].Position.xyz + size.x * x - size.y * y);
    position[3] = float3(input[0].Position.xyz + size.x * x + size.y * y);

    float2 uvs[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };
    GSOutput output;

    matrix global = input[0].Global;
   //[roll(4)]
    for (int i = 0; i < 4; i++)
    {
        output.Position = float4(position[i], 1);
        output.Position = mul(output.Position, global);
        output.Position = WorldPosition(output.Position);
        output.Position = ViewProjection(output.Position);
        output.Uv = uvs[i];
        output.color = input[0].color;
        stream.Append(output);
    }
}

float4 PSTexture(GSOutput input) : SV_Target0
{
    float4 diffuse = ParticleTex.Sample(LinearSampler, input.Uv);
    float4 color =input.color;
    color = (diffuse * color);
    return color;
}

float4 PSQuad(GSOutput input) : SV_Target0
{
    float4 color = input.color;

    [flatten]
    if (color.a < 0.1f)
        discard;
    return (color);
}
float4 PSDiamond(GSOutput input) : SV_Target0
{
    float2 uv = input.Uv*2;
    
    [flatten]
    if (uv.x > 1.0f)
        uv.x = 2.0f - uv.x;
    
    [flatten]
    if (uv.y > 1.0f)
        uv.y = 2.0f - uv.y;
    
    float dist2 = uv.x * uv.x + uv.y * uv.y;
    dist2 = saturate(dist2);
    float4 color    =input.color;
    color.a = lerp(0, color.a, dist2);    
    
    [flatten]
    if (color.a < 0.9f)
        discard;
    return color;
}

RasterizerState RS
{
    FillMode = Wireframe;
};

technique11 T0
{
    P_DSS_BS_VGP(P0, ParticleDSS, AlphaBlend, VS, GS, PSTexture)
    //P_RS_DSS_BS_VGP(P0,RS, ParticleDSS, AlphaBlend, VS, GS, PSTexture)
    P_DSS_BS_VGP(P1, ParticleDSS, AlphaBlend, VS, GS, PSQuad)
    P_DSS_BS_VGP(P2, ParticleDSS, AlphaBlend, VS, GS, PSDiamond)
}