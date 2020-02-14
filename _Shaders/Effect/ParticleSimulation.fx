#define MAX_INSTANCE 512
#define PI 3.1415926535897932384626433832795

struct ParticleInfo
{
    float4 StartColor;
    float4 EndColor;
    
    float Time;
    float DeltaTime;
    float Velocity;
    float Accelation;
		
    float RotVel;
    float RotAccel;
    float2 padding;
};
cbuffer CB_CS
{
    ParticleInfo info;
};

struct inputDesc
{
    float3  Postion;
    float3  Rotation;

    float3  Direction;
    float3  RotDir;

    float   LifeTime;
    float   Time;  
};
StructuredBuffer<inputDesc> InputDatas;

struct OutputDesc
{
    float3  Postion;
    float3  Rotation;
    float   LifeTime;
    float   Time;
    float4  ResultColor;
};
RWStructuredBuffer<OutputDesc> OutputDatas;


[numthreads(1024, 1, 1)]
void CS(uint GroupIndex : SV_GroupIndex, uint3 GroupID : SV_GroupID)
{
    uint    index       = (GroupID.x * 1024) + GroupIndex;
    float3  position    = InputDatas[index].Postion;
    float3  rotation    = InputDatas[index].Rotation;
    float3  dir         = InputDatas[index].Direction;
    float3  rotdir      = InputDatas[index].RotDir;
    float   lifespan    = InputDatas[index].LifeTime;
    float   life        = InputDatas[index].Time;
    
    
    float4 sColor   = info.StartColor;
    float4 eColor   = info.EndColor;
    
    float delta     = info.DeltaTime;
    float accel     = info.Accelation;
    float rotacc    = info.RotAccel;
    float vel       = info.Velocity;
    float rotvel    = info.RotVel;
    
    //life °è»ê
    life += delta;
    
    float   rate    = saturate(life / lifespan);
    float4  color   = lerp(sColor, eColor, rate);
    
    // v = v0+(a*t)/2
    vel += accel * life * 0.5f;
    rotvel += rotacc * life * 0.5f;
    
    position += vel * dir * delta;
    rotation += rotvel * rotdir * delta;
  
    
    OutputDatas[index].Postion      = position;
    OutputDatas[index].Rotation     = rotation;
    OutputDatas[index].Time         = life;
    OutputDatas[index].LifeTime     = lifespan;
    OutputDatas[index].ResultColor  = color;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CS()));
    }
}
