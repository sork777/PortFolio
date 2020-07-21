#include "../000_Header.fx"
#include "../000_Model.fx"
#include "../000_Light.fx"


///////////////////////////////////////////////////////////////////////////////
// 앞쪽이 기준 오브젝트 뒤쪽이 충돌한 오브젝트

bool Collision_OBBToOBB(CollisionOBB box1, CollisionOBB box2)
{
    float3 position = box2.Position - box1.Position;

	/* 각 박스의 면 법선벡터로 얻은 축 */
	if (SperatingPlane(position, box1.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box1.AxisZ, box1, box2) == true) return false;

	if (SperatingPlane(position, box2.AxisX, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisY, box1, box2) == true) return false;
	if (SperatingPlane(position, box2.AxisZ, box1, box2) == true) return false;

	/* box1과 box2의 Edge 하나씩을 Cross시켜 얻은 축 */
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
        만약 점이 Box 내부에 있다면
        리턴되는 q값과 일치해서 거리가 0일 거임
        외부에 있다면 q과 구와의 거리가 구의 반지름 이하일 경우 충돌
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
        1. 캡슐 양끝점을 잇는 선분
        2. OBB 중심에서 1.에 내린 수선의 발
        3. 1.2.의 교차점
        4. 3을 가지고 ClosestPtPointOBB 계산
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
        
    //OBB 중심에 가장 가까운 캡슐 선분 위의 점
    float3 dP = ClosestPtPointSegment(obb.Position, capS, capE);
    // dP와 가장 가까운 OBB 위의 점
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
    //if (dS < cap.CapRadius)      //캡슐 시작부분이 박스랑 충돌
    //    bCollision |= true;
    //else if (dE < cap.CapRadius) //캡슐 끝부분이 박스랑 충돌
    //    bCollision |= true;    
}


bool Collision_SphereToSphere(CollisionSphere sphere1, CollisionSphere sphere2)
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

bool Collision_SphereToCapsule(CollisionCapsule cap, CollisionSphere sphere)
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
        bCollision = true;
    
    return bCollision;
}


bool Collision_CapsuleToCapsule(CollisionCapsule cap1, CollisionCapsule cap2)
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
            
    // 출력인자는 들어올때 전부 0으로 초기화되는듯 하다. 비교 무의미
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

// 잘돌아감 0709
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

    //충돌 했으면 계산된 값, 아니면 FLT_MAX가 될거임
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
