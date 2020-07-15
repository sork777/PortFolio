struct Group
{
    //uint index;
    //uint3 id;
    //float Data;
    //float Data2;
    uint Index;
    uint3 GroupID;
    uint3 GroupThreadID;
    uint3 DispatchThreadID;
    uint GroupIndex;

};

StructuredBuffer<Group> Input;    //srv로 들어올애
RWStructuredBuffer<Group> Output; //uav랑 연결될 애

struct CS_Input
{
    /**/
    uint3 GroupID : SV_GroupID;
    uint3 GroupThreadID : SV_GroupThreadID;
    uint3 DispatchThreadID : SV_DispatchThreadID;
    uint GroupIndex : SV_GroupIndex;
};

[numthreads(4, 3, 2)]
void CS(CS_Input input)
{
    uint index = (input.GroupID.x * 24) + input.GroupIndex;

    //Output[index].index = index;
    //Output[index].id.x = input.DispatchThreadID.x;
    //Output[index].id.y = input.DispatchThreadID.y;
    //Output[index].id.z = input.DispatchThreadID.z;
    //Output[index].Data = Input[index].Data;
    //Output[index].Data2 = Input[index].Data* 10.0f;

    Output[index].Index = index;
    Output[index].GroupIndex = input.GroupIndex;
    Output[index].GroupID.x = input.GroupID.x;
    Output[index].GroupID.y = input.GroupID.y;
    Output[index].GroupID.z = input.GroupID.z;
    Output[index].GroupThreadID.x = input.GroupThreadID.x;
    Output[index].GroupThreadID.y = input.GroupThreadID.y;
    Output[index].GroupThreadID.z = input.GroupThreadID.z;
    Output[index].DispatchThreadID.x = input.DispatchThreadID.x;
    Output[index].DispatchThreadID.y = input.DispatchThreadID.y;
    Output[index].DispatchThreadID.z = input.DispatchThreadID.z;
 
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