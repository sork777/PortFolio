//=============================================================================================
// [Global Constant]
//=============================================================================================
#define PI 3.1415926535897932384626433832795
#define INV_PI 1.0 / PI
#define EPSILON 0.00000001




///////////////////////////////////////////////////////////////////////////////
// 행렬 분해, 쿼터니온과 행렬 변환
float4 MattoQuat(matrix input)
{
    float mat[16]
    =
    {
        input._11, input._12, input._13, input._14,
        input._21, input._22, input._23, input._24,
        input._31, input._32, input._33, input._34,
        input._41, input._42, input._43, input._44,
    };
    
    float T = 1 + mat[0] + mat[5] + mat[10];
    float S, X, Y, Z, W;
    
    [branch]
    if (T > 0.00000001)      //  to avoid    large distortions
    {
        S = sqrt(T) * 2;
        X = (mat[9] - mat[6]) / S;
        Y = (mat[2] - mat[8]) / S;
        Z = (mat[4] - mat[1]) / S;
        W = 0.25 * S;
    }
    else if (mat[0] > mat[5] && mat[0] > mat[10])
    { // Column 0:
        S = sqrt(1.0 + mat[0] - mat[5] - mat[10]) * 2;
        X = 0.25 * S;
        Y = (mat[4] + mat[1]) / S;
        Z = (mat[2] + mat[8]) / S;
        W = (mat[9] - mat[6]) / S;
    }
    else if (mat[5] > mat[10])
    { // Column 1: 
        S = sqrt(1.0 + mat[5] - mat[0] - mat[10]) * 2;
        X = (mat[4] + mat[1]) / S;
        Y = 0.25 * S;
        Z = (mat[9] + mat[6]) / S;
        W = (mat[2] - mat[8]) / S;
    }
    else
    { // Column 2: 
        S = sqrt(1.0 + mat[10] - mat[0] - mat[5]) * 2;
        X = (mat[2] + mat[8]) / S;
        Y = (mat[9] + mat[6]) / S;
        Z = 0.25 * S;
        W = (mat[4] - mat[1]) / S;
    }

    
    return float4(X, Y, Z, W);
}

matrix QuattoMat(float4 quat)
{
    float mat[16];
    
    float X = quat.x;
    float Y = quat.y;
    float Z = quat.z;
    float W = quat.w;
    
    float xx = X * X;
    float xy = X * Y;
    float xz = X * Z;
    float xw = X * W;
    float yy = Y * Y;
    float yz = Y * Z;
    float yw = Y * W;
    float zz = Z * Z;
    float zw = Z * W;
    
    mat[0] = 1 - 2 * (yy + zz);
    mat[1] = 2 * (xy - zw);
    mat[2] = 2 * (xz + yw);
    mat[4] = 2 * (xy + zw);
    mat[5] = 1 - 2 * (xx + zz);
    mat[6] = 2 * (yz - xw);
    mat[8] = 2 * (xz - yw);
    mat[9] = 2 * (yz + xw);
    mat[10] = 1 - 2 * (xx + yy);
    
    mat[3] = mat[7] = mat[11] = mat[12] = mat[13] = mat[14] = 0;
    mat[15] = 1;

    matrix result;
    result._11 = mat[0];
    result._12 = mat[1];
    result._13 = mat[2];
    result._14 = mat[3];
    result._21 = mat[4];
    result._22 = mat[5];
    result._23 = mat[6];
    result._24 = mat[7];
    result._31 = mat[8];
    result._32 = mat[9];
    result._33 = mat[10];
    result._34 = mat[11];
    result._41 = mat[12];
    result._42 = mat[13];
    result._43 = mat[14];
    result._44 = mat[15];
    return result;
}

///////////////////////////////////////////////////////////////////////////////

void MatrixDecompose(out matrix S, out float4 Q, out matrix T, matrix mat)
{
    S = T = 0;
    
    matrix R = mat;
    R._14 = R._24 = R._34 = R._41 = R._42 = R._43 = 0;
    
    float3 x = R._11_12_13;
    float3 y = R._21_22_23;
    float3 z = R._31_32_33;
    
    S._11 = length(x);
    S._22 = length(y);
    S._33 = length(z);
    S._44 = mat._44;
    
    Q = MattoQuat(R);
    
    
    T._11 = T._22 = T._33 = 1;
    T._41 = mat._41;
    T._42 = mat._42;
    T._43 = mat._43;
    T._44 = mat._44;
}

float CatMulRom_CalT(float ti, float3 Pi, float3 Pj)
{
    return pow(length(Pi - Pj), 0.5f) + ti;
}

float4 CatMulRom_CalPtoC(float4 P[4], float T[4], float t)
{
    float t0 = T[0];
    float t1 = T[1];
    float t2 = T[2];
    float t3 = T[3];
    
    float4 P0 = P[0];
    float4 P1 = P[1];
    float4 P2 = P[2];
    float4 P3 = P[3];
    
    float4 A1 = (t1 - t) / (t1 - t0) * P0 + (t - t0) / (t1 - t0) * P1;
    float4 A2 = (t2 - t) / (t2 - t1) * P1 + (t - t1) / (t2 - t1) * P2;
    float4 A3 = (t3 - t) / (t3 - t2) * P2 + (t - t2) / (t3 - t2) * P3;
    
    float4 B1 = (t2 - t) / (t2 - t0) * A1 + (t - t0) / (t2 - t0) * A2;
    float4 B2 = (t3 - t) / (t3 - t1) * A2 + (t - t1) / (t3 - t1) * A3;
    
    float4 C = (t2 - t) / (t2 - t1) * B1 + (t - t1) / (t2 - t1) * B2;
    
    return C;
}

///////////////////////////////////////////////////////////////////////////////
// 선분내에서 점과 가장 가까운 점을 리턴
float3 ClosestPtPointSegment(float3 pt, float3 segA, float3 segB)
{
    float t;
    float3 d;
    float3 segDir = segB - segA;
    t = dot((pt - segA), segDir);
    
    [branch]
    if(t<=0.0f)      //dot이 음수라 segA 앞쪽으로 수선의 발이 내림
    {
        t = 0.0f;
        d = segA;    //가장 가까운 지점은 segA
    }
    else
    {
        float denom = dot(segDir, segDir);    //선분의 길이 제곱.
        [branch]
        if(t>=denom)  //선분 길이를 벗어남 segB 뒤로 수선의 발
        {
            t = 1.0f;
            d = segB;
        }
        else          //선분 내에 안착
        {
            t /= denom;
            d = segA + t * segDir;
        }
    }
    return d;
}

void ClosestPtPointOBB(float3 pt, float3 obbCenter,float3 obbAxis[3], float obbSize[3], inout float3 q)
{
    float3 d = pt - obbCenter;
    q = obbCenter;
    [unroll(3)]
    for (int i = 0; i < 3;i++)
    {
        /*
            점과 박스의 센터를 연결한 선분을
            각 축에 투영시켜 해당 축으로 얼만큼 크기가 커져야 하는지 계산.
        */
        float dist = dot(d, obbAxis[i]);
        [flatten]
        if (dist > obbSize[i])
            dist = obbSize[i];
        [flatten]
        if (dist < -obbSize[i])
            dist = -obbSize[i];
        q += dist * obbAxis[i];
    }
}
void ClosestPtSegmentSegment(float3 segAS, float3 segAE, float3 segBS, float3 segBE, inout float3 c1, inout float3 c2)
{
    /*
        segA(s) = segAS + s * segA_Dir
        segB(t) = segBS + t * segB_Dir
        0 <= s,t <= 1
    */
    
    float s, t;
    float3 segA_Dir = segAE - segAS;
    float3 segB_Dir = segBE - segBS;
    float3 r = segAS - segBS;
    
    float segA_SqLen = dot(segA_Dir, segA_Dir);
    float segB_SqLen = dot(segB_Dir, segB_Dir);
    float f = dot(segB_Dir, r);

    //길이 없음이면 포인트인데...? 이것도 따지는듯.
    [flatten]
    if (segA_SqLen <= EPSILON && segB_SqLen <= EPSILON)
    {
        //서로가 점인경우
        s = t = 0.0f;
        c1 = segAS;
        c2 = segBS;
    }

    [branch]
    if (segA_SqLen <= EPSILON)
    {
        //A가 선분이 아니라 점인경우.
        s = 0.0f;
        t = f / segB_SqLen;
        t = clamp(t, 0.0f, 1.0f);
    }
    else
    {
        float c = dot(segA_Dir, r);
        if (segB_SqLen <= EPSILON)
        {
        //A가 선분이 아니라 점인경우.
            t = 0.0f;
            s = -c / segA_SqLen;
            s = clamp(s, 0.0f, 1.0f);
        }
        else
        {
            float b = dot(segA_Dir, segB_Dir);
        // (|A||B|)^2*(1-cos()^2) = (|A||B|)^2*sin()^2  즉 양수
            float denom = segA_SqLen * segB_SqLen - b * b;
        [branch]
            if (denom <= EPSILON)
            {
                s = 0.0f;
            }
            else
            {
                s = b * f - c * segB_SqLen;
                s /= denom;
                s = clamp(s, 0.0f, 1.0f);
            }
        
            t = (b * s + f) / segB_SqLen;
        
        [branch]
            if (t <= 0.0f)
            {
                t = 0.0f;
                s = -c / segA_SqLen;
                s = clamp(s, 0.0f, 1.0f);
            }
            else if (t >= 1.0f)
            {
                t = 1.0f;
                s = (b - c) / segA_SqLen;
                s = clamp(s, 0.0f, 1.0f);
            }
        }
    }
    
    c1 = segAS + s * segA_Dir;
    c2 = segBS + t * segB_Dir;
}

///////////////////////////////////////////////////////////////////////////////
// Collision
struct CollisionOBB
{
    float3 Position;

    float3 AxisX;
    float3 AxisY;
    float3 AxisZ;

    float3 HalfSize;
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

CollisionOBB MatrixtoOBB(matrix world)
{
    CollisionOBB output;
    
    float3 x = world._11_12_13;
    float3 y = world._21_22_23;
    float3 z = world._31_32_33;
    
    output.AxisX = x;
    output.AxisY = y;
    output.AxisZ = z;
    
    output.HalfSize.x = length(x);
    output.HalfSize.y = length(y);
    output.HalfSize.z = length(z);
    
    output.Position.x = world._41;
    output.Position.y = world._42;
    output.Position.z = world._43;
    
    return output;
}


int SperatingPlane(float3 position, float3 direction, CollisionOBB box1, CollisionOBB box2)
{
    float val = abs(dot(position, direction));

    float val2 = 0.0f;
    val2 += abs(dot((box1.AxisX * box1.HalfSize.x), direction));
    val2 += abs(dot((box1.AxisY * box1.HalfSize.y), direction));
    val2 += abs(dot((box1.AxisZ * box1.HalfSize.z), direction));
    val2 += abs(dot((box2.AxisX * box2.HalfSize.x), direction));
    val2 += abs(dot((box2.AxisY * box2.HalfSize.y), direction));
    val2 += abs(dot((box2.AxisZ * box2.HalfSize.z), direction));

    return (val > val2) ? 1 : 0;
}

///////////////////////////////////////////////////////////////////////////////
