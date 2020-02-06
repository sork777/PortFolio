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

void DivideMat(out matrix S, out float4 Q, out matrix T, matrix mat)
{
    S = T = 0;
    S._11 = mat._14;
    S._22 = mat._24;
    S._33 = mat._34;
    S._44 = mat._44;
    
    matrix R = mat;
    R._14 = R._24 = R._34 = R._41 = R._42 = R._41 = 0;
    //회전행렬의 쿼터니온 변화
    Q = MattoQuat(R);
        
    T._11 = T._22 = T._33 = 1;
    T._41 = mat._41;
    T._42 = mat._42;
    T._43 = mat._43;
    T._44 = mat._44;
}

///////////////////////////////////////////////////////////////////////////////

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
