
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

#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_INSTANCE 500
#define MAX_MODEL_KEYFRAMES 500

Texture2DArray TransformsMap;
Texture2D AnimEditTransformMap;

#include "000_Math.fx"

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

uint BoneIndex;

[numthreads(MAX_MODEL_INSTANCE, 1, 1)]
void CS(uint GroupIndex : SV_GroupIndex)
{
    // 인스턴싱 번호
    uint index = GroupIndex;
    uint boneIndex = BoneIndex;
    

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
            
        }
         //클립간 보간
        {
            //DivideMat(cS, cQ, cT, currAnim);
            //MatrixDecompose(cS, cQ, cT, currAnim);
            MatrixDecompose(cS, cQ, cT, currAnim);
            MatrixDecompose(nS, nQ, nT, nextAnim);
            cS = lerp(cS, nS, Tweenframes[index].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[index].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[index].TweenTime);
            
            currAnim = mul(cS, cR);
            currAnim = mul(currAnim, cT);
        
            
            MatrixDecompose(cS, cQ, cT, cedit);
            MatrixDecompose(nS, nQ, nT, nedit);
            cS = lerp(cS, nS, Tweenframes[index].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[index].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[index].TweenTime);
            
            cedit = mul(cS, cR);
            cedit = mul(cedit, cT);
            currAnim = mul(currAnim, cedit);
        }
    }
   
    
    Output[index].Result = currAnim;
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