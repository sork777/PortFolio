struct inputDesc
{
    uint index;
    float3 V0;
    float3 V1;
    float3 V2;
};
StructuredBuffer<inputDesc> Input;

struct OutputDesc
{
    uint Picked;
    float U;
    float V;
    float Distance;
};
RWStructuredBuffer<OutputDesc> Output;

cbuffer CB_Ray
{
    float3 Position;
    float Padding;

    float3 Direction;
};

void Intersection(uint index)
{
    float3 A = Input[index].V0;
    float3 B = Input[index].V1;
    float3 C = Input[index].V2;

    float3 e1 = B - A;
    float3 e2 = C - A;

    float3 P, T, Q;
    P = cross(Direction, e2);

    float d = 1.0f / dot(e1, P);

    T = Position - A;
    Output[index].U = dot(T, P) * d;

    Q = cross(T, e1);
    Output[index].V = dot(Direction, Q) * d;
    Output[index].Distance = dot(e2, Q) * d;

    bool b = (Output[index].U >= 0.0f) &&
                (Output[index].V >= 0.0f) &&
                (Output[index].U + Output[index].V <= 1.0f) &&
                (Output[index].Distance >= 0.0f);

    Output[index].Picked = b ? 1 : 0;

    //Output[index].U = index;
    //Output[index].V = Input[index].V0.x;
    //Output[index].Distance = Input[index].V0.z;
    
}


[numthreads(8, 8, 8)]
void CS(uint3 GroupID : SV_GroupID, uint GroupIndex : SV_GroupIndex)
{
    uint index = (GroupID.x * 512) + GroupIndex;
    Intersection(index);
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