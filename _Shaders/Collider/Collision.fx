#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"

struct CollisionOBB
{
    float3 OBBPosition;

    float3 OBBAxisX;
    float3 OBBAxisY;
    float3 OBBAxisZ;

    float3 OBBHalfSize;
};

struct CollisionCapsule
{
    float3 CapStart;
    float CapRadius;
    float3 CapDir;
    float CapHeight;
};

struct CollisionSphere
{
    float3 SpherePos;
    float SphereRadius;
};

bool Check_OBBToOBB(CollisionOBB obb1, CollisionOBB obb2)
{
    bool bCollision = false;
    
    return bCollision;
}
bool Check_OBBToSphere(CollisionOBB obb, CollisionSphere sphere)
{ 
    bool bCollision = false;
    /*
        만약 점이 Box 내부에 있다면
        리턴되는 q값과 일치해서 거리가 0일 거임
        외부에 있다면 q과 구와의 거리가 구의 반지름 이하일 경우 충돌
    */
    float3 axis[3];
    float size[3];
    axis[0] = obb.OBBAxisX;
    axis[1] = obb.OBBAxisY;
    axis[2] = obb.OBBAxisZ;
    size[0] = obb.OBBHalfSize.x;
    size[1] = obb.OBBHalfSize.y;
    size[2] = obb.OBBHalfSize.z;
    float3 q;
    ClosestPtPointOBB(sphere.SpherePos, obb.OBBPosition, axis, size, q);
    
    float3 d = length(q - sphere.SpherePos);
    
    [flatten]
    if(d < sphere.SphereRadius)
        bCollision = true;
    
    return bCollision;
}

bool Check_OBBToCapsule(CollisionOBB obb, CollisionCapsule cap)
{
    bool bCollision = false;
    
    float3 capS = cap.CapStart;
    float3 capE = (cap.CapStart + cap.CapDir * cap.CapHeight);
    
    float3 axis[3];
    float size[3];
    axis[0] = obb.OBBAxisX;
    axis[1] = obb.OBBAxisY;
    axis[2] = obb.OBBAxisZ;
    size[0] = obb.OBBHalfSize.x;
    size[1] = obb.OBBHalfSize.y;
    size[2] = obb.OBBHalfSize.z;
    float3 qS,qE;
    ClosestPtPointOBB(capS, obb.OBBPosition, axis, size, qS);
    ClosestPtPointOBB(capE, obb.OBBPosition, axis, size, qE);
    float3 dS = length(qS - capS);
    float3 dE = length(qE - capE);
    
    
    [branch]
    if (dS < cap.CapRadius)      //캡슐 시작부분이 박스랑 충돌
        bCollision |= true;
    else if (dE < cap.CapRadius) //캡슐 끝부분이 박스랑 충돌
        bCollision |= true;
    
    return bCollision;
}

bool Check_SphereToSphere(CollisionSphere sphere1, CollisionSphere sphere2)
{
    bool bCollision = false;
    /*
        구 중심간의 거리가 각 구의 반지름 합산보다 작으면 충돌
    */
    float MaxD = sphere1.SphereRadius + sphere2.SphereRadius;
    float3 CtoC = sphere1.SpherePos - sphere2.SpherePos;
    float d = length(CtoC);
    
    [flatten]
    if (d <= MaxD)
        bCollision = true;
    
    return bCollision;
}

bool Check_SphereToCapsule(CollisionCapsule cap, CollisionSphere sphere)
{
    bool bCollision = false;
     
    /*
        캡슐 기둥(선분)과 구(점)사이의 최단 거리 측정
        그 값이 측정 반지름 합산보다 작으면 충돌
    */
    
    // 캡슐은 구가 Dir으로 Height만큼 움직인 형태
    // 구와 캡슐 사이의 최대 거리
    float MaxD = sphere.SphereRadius + cap.CapRadius;
    float3 capS = cap.CapStart;
    float3 capE = (cap.CapStart + cap.CapDir * cap.CapHeight);  
    //캡슐기둥중심(선분)에서 구의중심(점)에 가장 가까운 점.
    float3 d = ClosestPtPointSegment(sphere.SpherePos, capS, capE);
    
    [branch]
    if (length(d) <= MaxD)
        bCollision |= true;
    
    return bCollision;
}

bool Check_CapsuleToCapsule(CollisionCapsule cap1, CollisionCapsule cap2)
{
    bool bCollision = false;
    
    /*
        선분 끼리의 최소 거리를 계산
        해당 길이가 서로간의 반지름을 합한 값보다 작으면 충돌.
    */
        
    float MaxD = cap2.CapRadius + cap1.CapRadius;

    //각 캡슐 시작점    
    float3 cap1S = cap1.CapStart;
    float3 cap2S = cap2.CapStart;
    //각 캡슐 끝 점
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

technique11 T0
{
}