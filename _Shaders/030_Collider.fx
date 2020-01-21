
struct ClipFrameBoneMatrix
{
    matrix Bone;
};

struct ResultMatrix
{
    matrix Result;
};
StructuredBuffer<ClipFrameBoneMatrix> Input;
RWStructuredBuffer<ResultMatrix> Output;

Texture2DArray TransformsMap;
Texture2D AnimEditTransformMap;

#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_INSTANCE 500
#define MAX_MODEL_KEYFRAMES 500
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
struct AnimationFrame
{
    int Clip;

    uint CurrFrame;
    uint NextFrame;

    float Time;
    float Running;

    float3 Padding;
};

struct TweenFrame
{
    float TakeTime;
    float TweenTime;
    float RunningTime;
    float Padding;

    AnimationFrame Curr;
    AnimationFrame Next;
};

cbuffer CB_AnimationFrame
{
    TweenFrame Tweenframes[MAX_MODEL_INSTANCE];
};


[numthreads(MAX_MODEL_TRANSFORMS, 1, 1)]
void CS(uint GroupIndex : SV_GroupIndex, uint3 GroupID : SV_GroupID)
{
    uint index = GroupID.x;
    uint boneIndex = GroupIndex;
    

    matrix result = 0;
    matrix curr = 0, currAnim = 0;
    matrix next = 0, nextAnim = 0;
    
    uint clip[2];
    uint currFrame[2];
    uint nextFrame[2];
    float time[2];

    clip[0] = Tweenframes[index].Curr.Clip;
    currFrame[0] = Tweenframes[index].Curr.CurrFrame;
    nextFrame[0] = Tweenframes[index].Curr.NextFrame;
    time[0] = Tweenframes[index].Curr.Time;

    clip[1] = Tweenframes[index].Next.Clip;
    currFrame[1] = Tweenframes[index].Next.CurrFrame;
    nextFrame[1] = Tweenframes[index].Next.NextFrame;
    time[1] = Tweenframes[index].Next.Time;
    
    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;
    
    float4 b0, b1, b2, b3;
    matrix cedit = 0;
    matrix nedit = 0;
    matrix cS, cR, cT, nS, nR, nT;
    float4 cQ, nQ;
    
    //[unroll(4)]
    //for (int i = 0; i < 4; i++)
    {
        c0 = TransformsMap.Load(int4(boneIndex * 4 + 0, currFrame[0], clip[0], 0));
        c1 = TransformsMap.Load(int4(boneIndex * 4 + 1, currFrame[0], clip[0], 0));
        c2 = TransformsMap.Load(int4(boneIndex * 4 + 2, currFrame[0], clip[0], 0));
        c3 = TransformsMap.Load(int4(boneIndex * 4 + 3, currFrame[0], clip[0], 0));
        curr = matrix(c0, c1, c2, c3);

        n0 = TransformsMap.Load(int4(boneIndex * 4 + 0, nextFrame[0], clip[0], 0));
        n1 = TransformsMap.Load(int4(boneIndex * 4 + 1, nextFrame[0], clip[0], 0));
        n2 = TransformsMap.Load(int4(boneIndex * 4 + 2, nextFrame[0], clip[0], 0));
        n3 = TransformsMap.Load(int4(boneIndex * 4 + 3, nextFrame[0], clip[0], 0));
        next = matrix(n0, n1, n2, n3);

        currAnim = lerp(curr, next, time[0]);
               
        /* 애니메이션 변화 부분 */        
        b0 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 0, clip[0], 0));
        b1 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 1, clip[0], 0));
        b2 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 2, clip[0], 0));
        b3 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 3, clip[0], 0));
        
        cedit = matrix(b0, b1, b2, b3);
        
        [flatten]
        if (clip[1] >= 0)
        {
            c0 = TransformsMap.Load(int4(boneIndex * 4 + 0, currFrame[1], clip[1], 0));
            c1 = TransformsMap.Load(int4(boneIndex * 4 + 1, currFrame[1], clip[1], 0));
            c2 = TransformsMap.Load(int4(boneIndex * 4 + 2, currFrame[1], clip[1], 0));
            c3 = TransformsMap.Load(int4(boneIndex * 4 + 3, currFrame[1], clip[1], 0));
            curr = matrix(c0, c1, c2, c3);

            n0 = TransformsMap.Load(int4(boneIndex * 4 + 0, nextFrame[1], clip[1], 0));
            n1 = TransformsMap.Load(int4(boneIndex * 4 + 1, nextFrame[1], clip[1], 0));
            n2 = TransformsMap.Load(int4(boneIndex * 4 + 2, nextFrame[1], clip[1], 0));
            n3 = TransformsMap.Load(int4(boneIndex * 4 + 3, nextFrame[1], clip[1], 0));
            next = matrix(n0, n1, n2, n3);

            nextAnim = lerp(curr, next, time[1]);
            
            /* 애니메이션 변화 부분 */
            b0 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 0, clip[1], 0));
            b1 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 1, clip[1], 0));
            b2 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 2, clip[1], 0));
            b3 = AnimEditTransformMap.Load(int3(boneIndex * 4 + 3, clip[1], 0));
        
            nedit = matrix(b0, b1, b2, b3);
            
            //result = lerp(result, nextAnim, Tweenframes[index].TweenTime);
        }
         //클립간 보간
        {
            DivideMat(cS, cQ, cT, currAnim);
            DivideMat(nS, nQ, nT, nextAnim);
            cS = lerp(cS, nS, Tweenframes[index].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[index].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[index].TweenTime);
            
            currAnim = mul(cS, cR);
            currAnim = mul(currAnim, cT);
        
            
            DivideMat(cS, cQ, cT, cedit);
            DivideMat(nS, nQ, nT, nedit);
            cS = lerp(cS, nS, Tweenframes[index].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[index].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[index].TweenTime);
            
            cedit = mul(cS, cR);
            cedit = mul(cedit, cT);
            currAnim = mul(currAnim, cedit);
        }
    }
   
    
    Output[index * MAX_MODEL_TRANSFORMS + boneIndex].Result = currAnim;
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