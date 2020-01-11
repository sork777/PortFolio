#include "000_Header.fx"
#include "000_Light.fx"

cbuffer CB_Rain
{
    float4 Color;
    float3 Velocity;

    float DrawDistance;

    float3 Origin;
    float CB_Rain_Padding;

    float3 Extent;


};

//따로 정의한 인풋 데이터

struct VertexInput
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 Scale : Scale0;
};


struct VertexOutput
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float Alpha : Alpha0;
};
VertexOutput VS(VertexInput input)
{
    VertexOutput output;
    float3 velocity = Velocity;
    //스케일이 큰것은 빠르게(가까운것?)
    velocity.xz /= input.Scale.y * 0.1f;
    float3 displace = Time * velocity;
    //extent안으로
    //다떨어진 애를 올려주는 놈
    input.Position.xyz = Origin + (Extent + (input.Position.xyz + displace) % Extent - (Extent * 0.5f));
    
    float4 position = WorldPosition(input.Position);
    //떨어지는 방향의 반대
    float3 up = normalize(-velocity);
    float3 forward = position.xyz - ViewPosition();
    float3 right = normalize(cross(up, forward));
    
    position.xyz += (input.Uv.x - 0.5f) * right * input.Scale.x;
    position.xyz += (1.5f - input.Uv.y * 1.5f) * up * input.Scale.y;
    position.w = 1.0f;
    output.Position = ViewProjection(position);
    output.Uv = input.Uv;

    float alpha = cos(input.Position.x + input.Position.z + Time);
    alpha = saturate(alpha / DrawDistance * 2.0f + 1.5f);
    output.Alpha = 0.2f * saturate(1 - output.Position.z / DrawDistance)*alpha;
    return output;
}

float4 PS(VertexOutput input) : SV_Target0
{
    float4 color = DiffuseMap.Sample(LinearSampler, input.Uv);
    color.rgb += Color.rgb * (1 + input.Alpha) * 2.0f;
    color.a = color.a * (input.Alpha * 1.5f);
    return color;
}


technique11 T0
{
    P_BS_VP(P0, AlphaBlend, VS, PS)
}