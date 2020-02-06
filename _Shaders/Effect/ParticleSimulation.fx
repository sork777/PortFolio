#define MAX_INSTANCE 4096
#define PI 3.1415926535897932384626433832795

struct ParticleInfo
{
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
    float4  StartColor;
    float4  EndColor;
};
StructuredBuffer<inputDesc> InputDatas;

struct OutputDesc
{
    float3  Postion;
    float3  Rotation;
    float   LifeTime;
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
    float   life        = InputDatas[index].LifeTime;
    float4  sColor      = InputDatas[index].StartColor;
    float4  eColor      = InputDatas[index].EndColor;
    
    float lifespan  = info.Time;
    float delta     = info.DeltaTime;
    float accel     = info.Accelation;
    float rotacc    = info.RotAccel;
    float vel       = info.Velocity;
    float rotvel    = info.RotVel;
    
    //life °è»ê
    life += delta;
    
    float   rate    = life / lifespan;
    float4  color   = lerp(sColor, eColor, rate);
    
    // v = v0+(a*t)/2
    vel += accel * life * 0.5f;
    
    position += vel * dir;
    rotation += rotvel * rotdir;
    [flatten]
    if (rotation >= 2 * PI)
        rotation -= 2 * PI;
    [flatten]
    if (rotation <0)
        rotation += 2 * PI;
    
    OutputDatas[index].Postion      = position;
    OutputDatas[index].Rotation     = rotation;
    OutputDatas[index].LifeTime     = life;
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
