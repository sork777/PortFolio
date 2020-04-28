#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"

struct OBB_Input
{
    matrix data;
};
struct Sphere_Input
{
    float3 Pos;
    float Radius;
};
struct Cap_Input
{
    float3 Start;
    float Radius;
    float3 Dir;
    float Height;
};

StructuredBuffer<OBB_Input> OBB_Datas;
//StructuredBuffer<Sphere_Input> Sphere_Datas;
//StructuredBuffer<Cap_Input> Cap_Datas;

struct Col_Output
{
    int bCollsion;
    float3 MaxRound;
    float3 MinRound;
    float dist;
};

///////////////////////////////////////////////////////////////////////////////
// ������ ���� ������Ʈ ������ �浹�� ������Ʈ
// �浹 : OBB -1 Sph - 2 Cap - 4 �ջ�

bool Collision_OBBToOBB(CollisionOBB box1, CollisionOBB box2)
{
    float position = box2.Position - box1.Position;

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
    
    float3 d = length(q - sphere.SpherePos);
    
    [flatten]
    if(d < sphere.SphereRadius)
        bCollision = true;
    
    return bCollision;
}

bool Collision_OBBToCapsule(CollisionOBB obb, CollisionCapsule cap)
{
    bool bCollision = false;
    
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
    float3 qS,qE;
    ClosestPtPointOBB(capS, obb.Position, axis, size, qS);
    ClosestPtPointOBB(capE, obb.Position, axis, size, qE);
    float3 dS = length(qS - capS);
    float3 dE = length(qE - capE);
    
    
    [branch]
    if (dS < cap.CapRadius)      //ĸ�� ���ۺκ��� �ڽ��� �浹
        bCollision |= true;
    else if (dE < cap.CapRadius) //ĸ�� ���κ��� �ڽ��� �浹
        bCollision |= true;
    
    return bCollision;
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
        bCollision |= true;
    
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

technique11 T0
{
}