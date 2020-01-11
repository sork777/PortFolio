ByteAddressBuffer Input;    //srv로 들어올애
RWByteAddressBuffer Output;    //uav랑 연결될 애

struct Group
{
    uint3 GroupID;
    uint3 GroupThreadID;
    uint3 DispatchThreadID;
    uint GroupIndex;

   // float Random;
   // float RandomOutput;
};

struct CS_Input
{
    /**/
    uint3 GroupID : SV_GroupID;
    uint3 GroupThreadID : SV_GroupThreadID;
    uint3 DispatchThreadID : SV_DispatchThreadID;
    uint GroupIndex : SV_GroupIndex;
};

/*한 그룹내의 스레드 분할 갯수*/
//순서대로 x,y,z 최대는 512 넘지 안도록
[numthreads(10, 8, 3)]
void CS(CS_Input input)
{
    Group group;
    /*byte adrress라 타입에 맞게 캐스팅*/
    group.GroupID = asint(input.GroupID);
    group.GroupThreadID = asint(input.GroupThreadID);
    group.DispatchThreadID = asint(input.DispatchThreadID);
    group.GroupIndex = asint(input.GroupIndex);

    /* 원래는 y,z도 따로 계산 해야함*/
    uint index = input.GroupID.x* 240+input.GroupIndex;
    uint fetchAddress = index * 10 * 4;
    /* 
    input.load 가 4바이트 불러옴 
    해당 위치가 random임을 상기 
    */
   // group.Random = asfloat(Input.Load(fetchAddress + 40));
   // group.RandomOutput = group.Random * 10.0f;

    Output.Store3(fetchAddress + 0, asuint(group.GroupID));
    Output.Store3(fetchAddress + 12, asuint(group.GroupThreadID));
    Output.Store3(fetchAddress + 24, asuint(group.DispatchThreadID));
    Output.Store(fetchAddress + 36, asuint(group.GroupIndex));
    /* float이어도 내보낼때는 */
    //Output.Store(fetchAddress + 40, asuint(group.Random));
    //Output.Store(fetchAddress + 44, asuint(group.RandomOutput));
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