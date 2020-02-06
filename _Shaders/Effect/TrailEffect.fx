#include "../000_Header.fx"
//#include "../000_Model.fx"
//#include "../000_Light.fx"

#define MAX_TRAILBUFFER_COUNT 128

struct TrailDesc
{
    matrix TrailMatrix[MAX_TRAILBUFFER_COUNT];

    int TrailCount;
    int Segment;
    float2 VS_TrailMatrix_Padding;
};
cbuffer CB_Trail
{
    TrailDesc Trail;
};
Texture2D TrailTexture;
Texture2D MaskTexture;


struct TrailVSOutput
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
    uint Id : ID0;
};

TrailVSOutput VS_Trail(VertexTexture input)
{
    TrailVSOutput output;

    float rate = 1.0f / (float) Trail.TrailCount;
    int idx = (int) ((input.Uv.x) * Trail.TrailCount);
    idx = min(idx, Trail.TrailCount - 1);

    matrix mat = Trail.TrailMatrix[idx];

    //output.Position = input.Position;
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, mat);
    output.Position = ViewProjection(output.Position);

    output.Uv = input.Uv;
    output.Id = idx;
    return output;
}

TrailVSOutput VS_TrailGS(VertexTexture input)
{
    TrailVSOutput output;

    int idx = (int) ((input.Uv.x) * Trail.TrailCount);

    output.Id = min(idx, Trail.TrailCount - 1);
    output.Position = input.Position;

    output.Uv = input.Uv;

    return output;
}

struct TrailGSOutput
{
    float4 Position : SV_Position0;
    float2 Uv : Uv0;       
};

[maxvertexcount(128)]
void GS_Trail(point TrailVSOutput input[1], inout TriangleStream<TrailGSOutput> stream)
{
    
    float x = input[0].Position.x;
    float y = input[0].Position.y;
    
    float3 position[4];
    position[0] = float3(x - 0.5f, y - 0.5f, 0);
    position[1] = float3(x - 0.5f, y + 0.5f, 0);
    position[2] = float3(x + 0.5f, y - 0.5f, 0);
    position[3] = float3(x + 0.5f, y + 0.5f, 0);

    
    matrix S[4],R,T[4];
    float4 Q[4], Translation[4];
    matrix PMat[4];
    
    int   Idx;
    float Time[4];
    Time[0] = 0;    
    //[roll(4)]   
    for (int i = 0; i < 4; i++)
    {       
        Idx = input[0].Id + (i);
        Idx = min(Idx, Trail.TrailCount - 1);
        PMat[i] = Trail.TrailMatrix[Idx];
        DivideMat(S[i], Q[i], T[i], PMat[i]);
        Translation[i]  = T[i]._41_42_43_44;
    }
    
    //[roll(4)]   
    for (i = 1; i < 4; i++)
        Time[i] = CatMulRom_CalT(Time[i - 1], Translation[i].xyz, Translation[i - 1].xyz);
    float tstart    = Time[1];
    float tend      = Time[2];
    
    
    int segments  = Trail.Segment;    
    segments = max(2, segments);
    float f = 1 / segments;
    matrix sMat = 0;
    float rate = 0;
    float4 pos = 0;
    float4 q = 0;
    
    float ux = input[0].Uv.x;
    float2 uvs[4] =
    {
        float2(0, 1), float2(0, 0), float2(1, 1), float2(1, 0)
    };
    TrailGSOutput output;
    
    //[roll()]
    for (int j = 1; j < segments; j++)
    {
        rate = tstart + (j * (tend - tstart)) / segments;
        // Scale 보간
        sMat = lerp(S[1], S[2], rate);
        // 회전 보간
        q = normalize(CatMulRom_CalPtoC(Q, Time, rate));
        //q = normalize(lerp(Q[1], Q[2], rate));
        R = QuattoMat(q);
        // 위치 보간 - CatMullRom
        pos = CatMulRom_CalPtoC(Translation, Time, rate);
        sMat = mul(sMat, R);
        sMat._41 = pos.x;
        sMat._42 = pos.y;
        sMat._43 = pos.z;
         //[roll(4)]
        for (int k = 0; k < 4; k++)
        {
            output.Position = WorldPosition(float4(position[k], 1));
            output.Position = mul(output.Position, sMat);
            output.Position = ViewProjection(output.Position);
        
            output.Uv = uvs[k];
            output.Uv.x *= j/segments;
            output.Uv.x += ux;
        
            stream.Append(output);
        }
        //stream.RestartStrip();
        
    }    
}

float4 PS_Trail(TrailVSOutput input) : SV_Target0
{
    //input.Uv.y = 1.0f - input.Uv.y;
    float4 color = TrailTexture.Sample(LinearSampler, input.Uv);
    float4 Mask = MaskTexture.Sample(LinearSampler, input.Uv);
    color *= Mask;
    color.a = 1 - input.Uv.x;
    return color;
    //return float4(1, 0, 0, 1);
}

float4 PS_TrailGS(TrailGSOutput input) : SV_Target0
{
    //input.Uv.y = 1.0f - input.Uv.y;
    float4 color = TrailTexture.Sample(LinearSampler, input.Uv);
    float4 Mask = MaskTexture.Sample(LinearSampler, input.Uv);
    color *= Mask;
    color.a = 1 - input.Uv.x;
    return color;
}

RasterizerState TrailWRS
{
    FillMode = WireFrame;    
    CullMode = None;
};
technique11 TNormalRender
{
	//P_RS_BS_VP(P0, TrailRS, TrailBlend, VS_Trail,  PS_Trail)
	P_RS_BS_VGP(P1, TrailRS, TrailBlend, VS_TrailGS, GS_Trail, PS_TrailGS)
}
technique11 TWireFrame
{
//	P_RS_BS_VP(P0, TrailWRS, TrailBlend, VS_Trail,  PS_Trail)
	P_RS_BS_VGP(P1, TrailWRS, TrailBlend, VS_TrailGS, GS_Trail, PS_TrailGS)
}
