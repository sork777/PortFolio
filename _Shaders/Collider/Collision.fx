#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"


///////////////////////////////////////////////////////////////////////////////
// ������ ���� ������Ʈ ������ �浹�� ������Ʈ

bool Collision_OBBToOBB(CollisionOBB box1, CollisionOBB box2)
{
    float3 position = box2.Position - box1.Position;

	/* �� �ڽ��� �� �������ͷ� ���� �� */
	if (SperatingPlane(position, box1.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisZ, box1, box2) == true) return false;

	if (SperatingPlane(position, box2.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisZ, box1, box2) == true) return false;

	/* box1�� box2�� Edge �ϳ����� Cross���� ���� �� */
	if (SperatingPlane(position, cross(box1.AxisX, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisX, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisX, box2.AxisZ), box1, box2) == true) return false;

	if (SperatingPlane(position, cross(box1.AxisY, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisY, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisY, box2.AxisZ), box1, box2) == true) return false;
								 
	if (SperatingPlane(position, cross(box1.AxisZ, box2.AxisX), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisZ, box2.AxisY), box1, box2) == true) return false;
	if (SperatingPlane(position, cross(box1.AxisZ, box2.AxisZ), box1, box2) == true) return false;

    return true;
}

bool Collision_OBBToSphere(CollisionOBB obb, CollisionSphere sphere)
{ 
    bool bCollision = false;
    /*
        ���� ���� Box ���ο� �ִٸ�
        ���ϵǴ� q���� ��ġ�ؼ� �Ÿ��� 0�� ����
        �ܺο� �ִٸ� q�� ������ �Ÿ��� ���� ������ ������ ��� �浹
    */
    float3 axis[3];
    float size[3];
    axis[0] = obb.AxisX;
    axis[1] = obb.AxisY;
    axis[2] = obb.AxisZ;
    size[0] = obb.HalfSize.x;
    size[1] = obb.HalfSize.y;
    size[2] = obb.HalfSize.z;
    float3 q;
    ClosestPtPointOBB(sphere.SpherePos, obb.Position, axis, size, q);
    
    float d = length(q - sphere.SpherePos);
    
    [flatten]
    if(d < sphere.SphereRadius)
        bCollision = true;
    
    return bCollision;
}

bool Collision_OBBToCapsule(CollisionOBB obb, CollisionCapsule cap)
{
    bool bCollision = false;
    /*
        1. ĸ�� �糡���� �մ� ����
        2. OBB �߽ɿ��� 1.�� ���� ������ ��
        3. 1.2.�� ������
        4. 3�� ������ ClosestPtPointOBB ���
    */
    float3 capS = cap.CapStart;
    float3 capE = (cap.CapStart + cap.CapDir * cap.CapHeight);
    
    float3 axis[3];
    float size[3];
    axis[0] = obb.AxisX;
    axis[1] = obb.AxisY;
    axis[2] = obb.AxisZ;
    size[0] = obb.HalfSize.x;
    size[1] = obb.HalfSize.y;
    size[2] = obb.HalfSize.z;
        
    //OBB �߽ɿ� ���� ����� ĸ�� ���� ���� ��
    float3 dP = ClosestPtPointSegment(obb.Position, capS, capE);
    // dP�� ���� ����� OBB ���� ��
    float3 qP;
    ClosestPtPointOBB(dP, obb.Position, axis, size, qP);
    float d = length(qP - dP);
    
    if (d < cap.CapRadius)
        bCollision = true;
    
    return bCollision;
    
    //float3 qS,qE;
    //ClosestPtPointOBB(capS, obb.Position, axis, size, qS);
    //ClosestPtPointOBB(capE, obb.Position, axis, size, qE);
    //float3 dS = length(qS - capS);
    //float3 dE = length(qE - capE);
    
    
    //[branch]
    //if (dS < cap.CapRadius)      //ĸ�� ���ۺκ��� �ڽ��� �浹
    //    bCollision |= true;
    //else if (dE < cap.CapRadius) //ĸ�� ���κ��� �ڽ��� �浹
    //    bCollision |= true;    
}


bool Collision_SphereToSphere(CollisionSphere sphere1, CollisionSphere sphere2)
{
    bool bCollision = false;
    /*
        �� �߽ɰ��� �Ÿ��� �� ���� ������ �ջ꺸�� ������ �浹
    */
    float MaxD = sphere1.SphereRadius + sphere2.SphereRadius;
    float3 CtoC = sphere1.SpherePos - sphere2.SpherePos;
    float d = length(CtoC);
    
    [flatten]
    if (d <= MaxD)
        bCollision = true;
    
    return bCollision;
}

bool Collision_SphereToCapsule(CollisionCapsule cap, CollisionSphere sphere)
{
    bool bCollision = false;
     
    /*
        ĸ�� ���(����)�� ��(��)������ �ִ� �Ÿ� ����
        �� ���� ���� ������ �ջ꺸�� ������ �浹
    */
    
    // ĸ���� ���� Dir���� Height��ŭ ������ ����
    // ���� ĸ�� ������ �ִ� �Ÿ�
    float MaxD = sphere.SphereRadius + cap.CapRadius;
    float3 capS = cap.CapStart;
    float3 capE = (cap.CapStart + cap.CapDir * cap.CapHeight);  
    //ĸ������߽�(����)���� �����߽�(��)�� ���� ����� ��.
    float3 d = ClosestPtPointSegment(sphere.SpherePos, capS, capE);
    
    [branch]
    if (length(d) <= MaxD)
        bCollision = true;
    
    return bCollision;
}


bool Collision_CapsuleToCapsule(CollisionCapsule cap1, CollisionCapsule cap2)
{
    bool bCollision = false;
    
    /*
        ���� ������ �ּ� �Ÿ��� ���
        �ش� ���̰� ���ΰ��� �������� ���� ������ ������ �浹.
    */
        
    float MaxD = cap2.CapRadius + cap1.CapRadius;

    //�� ĸ�� ������    
    float3 cap1S = cap1.CapStart;
    float3 cap2S = cap2.CapStart;
    //�� ĸ�� �� ��
    float3 cap1E = cap1.CapStart + cap1.CapDir * cap1.CapHeight;
    float3 cap2E = cap2.CapStart + cap2.CapDir * cap2.CapHeight;
    float3 c1, c2;
    ClosestPtSegmentSegment(cap1S, cap1E, cap2S, cap2E, c1, c2);
    
    float d = length(c1 - c2);
    
    [flatten]
    if (d <= MaxD)
        bCollision = true;
    
    return bCollision;
}
///////////////////////////////////////////////////////////////////////////////

struct Col_Input
{
    matrix data;
};

StructuredBuffer<Col_Input> Col_InputsA;
StructuredBuffer<Col_Input> Col_InputsB;

struct Col_Output
{
    int bCollsion;
    float Dist;
    int ClosestNum;
    int bFrustum;
};
RWStructuredBuffer<Col_Output> Col_Outputs;

int SelfCol;
int Col_Size;

bool InFrustum(float3 center)
{
    [roll(6)]
    for (int i = 0; i < 6; i++)
    {
        float4 plane = Planes[i];
        if(!any(plane))
            continue;
        float3 n = normalize(plane.xyz);
        float s = dot(float4(center, 1), plane);
        [flatten]
        if (s<0.0f)
        {
            return false;
        }        
    }
    return true;
}

[numthreads(1024, 1, 1)]
void CS_OBBtoOBB(uint GroupIndex : SV_GroupIndex,uint GroupID:SV_GroupID)
{
    uint colA_ID = GroupIndex+1024*GroupID.x;
    
    CollisionOBB colA, colB;
    colA = MatrixtoOBB(Col_InputsA[colA_ID].data);
    bool finalCol = false;
    float closestdist = FLT_MAX;
    int closestN = -1;
    //[unroll()]
    for (int colB_ID = 0; colB_ID < Col_Size; colB_ID++)
    {
        float4x4 data = lerp(Col_InputsB[colB_ID].data, Col_InputsA[colB_ID].data, SelfCol);
        colB = MatrixtoOBB(data);
        bool bSameNum = SelfCol ? (colA_ID == colB_ID) : false;
        bool bCollide = bSameNum ? false : Collision_OBBToOBB(colA, colB);
        
        float dist = bCollide ? length(colA.Position - colB.Position):FLT_MAX;
        
        finalCol = finalCol | bCollide;
        if (closestdist > dist)
        {
            closestdist = dist;
            closestN = colB_ID;
        }

    }
            
    // ������ڴ� ���ö� ���� 0���� �ʱ�ȭ�Ǵµ� �ϴ�. �� ���ǹ�
    Col_Outputs[colA_ID].bCollsion = finalCol;
    Col_Outputs[colA_ID].Dist = closestdist;
    Col_Outputs[colA_ID].ClosestNum = closestN;
    Col_Outputs[colA_ID].bFrustum = InFrustum(colA.Position);
    
}
///////////////////////////////////////////////////////////////////////////////

struct MouseRay
{
    float3 RayPos;
    float padding;
    float3 RayDir;
    float padding2;
};
cbuffer CB_Ray
{
    MouseRay M_Ray;
};

// �ߵ��ư� 0709
[numthreads(1024, 1, 1)]
void CS_RaytoOBB(uint GroupIndex : SV_GroupIndex, uint3 GroupID : SV_GroupID)
{
    uint col_ID = GroupIndex + 1024 * GroupID.x;
    
    CollisionOBB obb;
    obb = MatrixtoOBB(Col_InputsA[col_ID].data);
    
    float3 MaxRound = obb.Position +
		obb.AxisX * obb.HalfSize.x +
		obb.AxisY * obb.HalfSize.y +
		obb.AxisZ * obb.HalfSize.z;
    float3 MinRound = obb.Position -
		obb.AxisX * obb.HalfSize.x -
		obb.AxisY * obb.HalfSize.y -
		obb.AxisZ * obb.HalfSize.z;
    
    float dist = FLT_MAX;
    bool bCollide = RayInterSection(M_Ray.RayPos, M_Ray.RayDir, MinRound,MaxRound, dist);

    //�浹 ������ ���� ��, �ƴϸ� FLT_MAX�� �ɰ���
    Col_Outputs[col_ID].bCollsion = bCollide ? 1 : 0;
    Col_Outputs[col_ID].Dist = dist;
    Col_Outputs[col_ID].bFrustum = InFrustum(obb.Position);

}
technique11 T_OBB
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CS_OBBtoOBB()));
    }
}

technique11 T_Ray
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, CS_RaytoOBB()));
    }
}
