#include "000_Header.fx"
#include "000_Light.fx"

struct VertexOutput
{
    float4 Position : Position0;  
};

VertexOutput VS(Vertex input)
{
    VertexOutput output;
    output.Position = input.Position;

    return output;
}

struct AppendGeometryOutput
{
    float4 Position : SV_Position0;
};

//Max_Vertex_Count
[maxvertexcount(14)]                 
void GS(triangle VertexOutput input[3], inout TriangleStream<AppendGeometryOutput> stream)
{
    //VertexOutput v[6];
    //VertexOutput p[3];

    //p[0].Position = (input[0].Position + input[1].Position) * 0.5f;
    //p[1].Position = (input[1].Position + input[2].Position) * 0.5f;
    //p[2].Position = (input[2].Position + input[0].Position) * 0.5f;

    //v[0] = input[0];
    //v[1] = p[0];
    //v[2] = p[2];
    //v[3] = p[1];
    //v[4] = input[2];
    //v[5] = input[1];

    //float4 position = 0;
    //AppendGeometryOutput output[6];
    ////[unroll(6)]
    //for (int i = 0; i < 6; i++)
    //{
    //    position = WorldPosition(v[i].Position);
    //    position = ViewProjection(position);
    //    output[i].Position = position;
    //    }
    ////쉐이더는 전역이라 
    ////[unroll(6)]
    //for (int k = 0; k < 5; k++)
    //    stream.Append(output[k]);

    ////TriangleStrip 상태라 끊을거임
    //stream.RestartStrip();

    //stream.Append(output[1]);
    //stream.Append(output[5]);
    //stream.Append(output[3]);

    VertexOutput p[8];
    p[0].Position = input[0].Position + float4(0, 0, -1, 0);	
    p[1].Position = input[0].Position + float4(0, 0, +1, 0);    
    p[2].Position = input[2].Position + float4(0, 0, -1, 0);
    p[3].Position = input[2].Position + float4(0, 0, +1, 0);
    p[4].Position = p[0].Position;    
    p[5].Position = p[1].Position;
    p[6].Position = p[2].Position;
    p[7].Position = p[3].Position;
    p[4].Position.y = input[1].Position.y;
    p[5].Position.y = input[1].Position.y;
    p[6].Position.y = input[1].Position.y;
    p[7].Position.y = input[1].Position.y;

    float4 position = 0;
    AppendGeometryOutput output[8];

    //[unroll(6)]
    for (int i = 0; i < 8; i++)
    {
        position = WorldPosition(p[i].Position);
        position = ViewProjection(position);
        output[i].Position = position;
    }
	/* 한붓 그리기! */
    stream.Append(output[0]);
    stream.Append(output[4]);
    stream.Append(output[2]);
    stream.Append(output[6]);
    stream.Append(output[7]);
    stream.Append(output[4]);
    stream.Append(output[5]);
    stream.Append(output[0]);
    stream.Append(output[1]);
    stream.Append(output[2]);
    stream.Append(output[3]);
    stream.Append(output[7]);
    stream.Append(output[1]);
    stream.Append(output[5]);

}


float4 PS(VertexOutput input) : SV_Target0
{
    return float4(1, 0, 0, 0);
}

RasterizerState RS
{
    Fillmode = Wireframe;
    Cullmode = None;
};
technique11 T0
{
    P_RS_VGP(P0, RS,VS, GS, PS)
}