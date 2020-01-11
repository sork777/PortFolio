ByteAddressBuffer Input;    //srv�� ���þ�
RWByteAddressBuffer Output;    //uav�� ����� ��

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

/*�� �׷쳻�� ������ ���� ����*/
//������� x,y,z �ִ�� 512 ���� �ȵ���
[numthreads(10, 8, 3)]
void CS(CS_Input input)
{
    Group group;
    /*byte adrress�� Ÿ�Կ� �°� ĳ����*/
    group.GroupID = asint(input.GroupID);
    group.GroupThreadID = asint(input.GroupThreadID);
    group.DispatchThreadID = asint(input.DispatchThreadID);
    group.GroupIndex = asint(input.GroupIndex);

    /* ������ y,z�� ���� ��� �ؾ���*/
    uint index = input.GroupID.x* 240+input.GroupIndex;
    uint fetchAddress = index * 10 * 4;
    /* 
    input.load �� 4����Ʈ �ҷ��� 
    �ش� ��ġ�� random���� ��� 
    */
   // group.Random = asfloat(Input.Load(fetchAddress + 40));
   // group.RandomOutput = group.Random * 10.0f;

    Output.Store3(fetchAddress + 0, asuint(group.GroupID));
    Output.Store3(fetchAddress + 12, asuint(group.GroupThreadID));
    Output.Store3(fetchAddress + 24, asuint(group.DispatchThreadID));
    Output.Store(fetchAddress + 36, asuint(group.GroupIndex));
    /* float�̾ ���������� */
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