
///////////////////////////////////////////////////////////////////////////////
// 매크로들 
#define VS_GENERATE \
output.oPosition = input.Position.xyz; \
output.Position = WorldPosition(input.Position); \
output.wPosition = output.Position.xyz; \
\
output.Position = ViewProjection(output.Position); \
output.wvpPosition = output.Position; \
output.Normal = WorldNormal(input.Normal); \
output.Tangent = WorldTangent(input.Tangent); \
output.Uv = input.Uv;
///////////////////////////////////////////////////////////////////////////////

struct VertexMesh
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;

    matrix Transform : InstTransform;    
};

void SetMeshWorld(inout matrix world, VertexMesh input)
{
    world = input.Transform;
}

MeshOutput VS_Mesh(VertexMesh input)
{
    MeshOutput output;
    
    SetMeshWorld(World, input);
    VS_GENERATE
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    output.Clip = 0;
    output.ID = 0;
    return output;

}

MeshTessOutput VS_Tess_Mesh(VertexMesh input)
{
    MeshTessOutput output;
    
    SetMeshWorld(World, input);
    output.Position = WorldPosition(input.Position);
    output.Normal = WorldNormal(input.Normal);
    output.Tangent = WorldTangent(input.Tangent);
    output.Uv = input.Uv;
    return output;

}

MeshOutput VS_Mesh_GS(VertexMesh input)
{
    MeshOutput output;
    
    VS_GENERATE
    
    output.wvpPosition = 0;

    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);

    output.Clip = 0;


    return output;

}
///////////////////////////////////////////////////////////////////////////////

DepthOutput VS_Depth_Mesh(VertexMesh input)
{
    DepthOutput output;
     
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, ShadowView);
    output.Position = mul(output.Position, ShadowProjection);
    output.sPosition = output.Position;
    
    return output;
}

float4 PS_Depth(DepthOutput input) : SV_Target0
{
    float depth = input.sPosition.z / input.sPosition.w;
    return float4(depth, depth, depth, 1);
    //return float4(final, final, final, 1);
}


///////////////////////////////////////////////////////////////////////////////

#define MAX_MODEL_INSTANCE 500
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
//상수로 크기를 가변으로 받아줄수 없어서?

Texture2DArray TransformsMap;
Texture1D BoneTransformsMap;
Texture2D AnimEditTransformMap;
//애니메이션하고 통합해서 빌드 할거임?
//인스터싱을 대비?
struct VertexModel
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float3 Normal : Normal0;
    float3 Tangent : Tangent0;
    float4 BlendIndices : BlendIndices0;
    float4 BlendWeights : BlendWeights0;

    matrix Transform : Inst0;
    uint InstID : SV_InstanceID;
};

cbuffer CB_Bone
{
    uint BoneIndex;
};

////////////////////////////////////////////////////////////////////////////////
/* 정지 모델 */
//inout : c의 레퍼런스와 같은 놈?
void SetModelWorld(inout matrix world, VertexModel input)
{
    float4 c0, c1, c2, c3;
    matrix anim=0;
    matrix transform=0;
    matrix curr = 0;
   

    {
        c0 = TransformsMap.Load(uint4(BoneIndex * 4 + 0, input.InstID, 0, 0));
        c1 = TransformsMap.Load(uint4(BoneIndex * 4 + 1, input.InstID, 0, 0));
        c2 = TransformsMap.Load(uint4(BoneIndex * 4 + 2, input.InstID, 0, 0));
        c3 = TransformsMap.Load(uint4(BoneIndex * 4 + 3, input.InstID, 0, 0));
        curr = matrix(c0, c1, c2, c3);

        transform = curr;
    }
    
    world = mul(transform, input.Transform);
}


MeshOutput VS_Model(VertexModel input)
{
    MeshOutput output;

    SetModelWorld(World, input);

    VS_GENERATE
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.ID = input.InstID;
    output.Clip = 0;

    return output;

}

MeshOutput VS_Model_GS(VertexModel input)
{
    MeshOutput output;
    SetModelWorld(World, input);
    VS_GENERATE

    output.wvpPosition = 0;
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);

    output.Clip = 0;
    return output;
}


DepthOutput VS_Depth_Model(VertexModel input)
{
    DepthOutput output;

    SetModelWorld(World, input);
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, ShadowView);
    output.Position = mul(output.Position, ShadowProjection);
    output.sPosition = output.Position;
    
    return output;
}

////////////////////////////////////////////////////////////////////////////////
struct Keyframe
{
    int Clip;

    uint CurrFrame;
    uint NextFrame;

    float Time;
    float RunningTime;
    float Speed;

    float2 Padding;
};

struct TweenFrame
{
    float TweenRunningTime;
    float TakeTweenTime;
    float TweenTime;
    float Padding;

    Keyframe Curr;
    Keyframe Next;
};

cbuffer CB_AnimationFrame
{
    TweenFrame Tweenframes[MAX_MODEL_INSTANCE];
};

void SetAnimationWorld(inout matrix world, VertexModel input)
{
    matrix transform = 0;
    matrix curr = 0, currAnim = 0;
    matrix next = 0, nextAnim = 0;
    
    float4 c0, c1, c2, c3;
    float4 n0, n1, n2, n3;
    float4 b0, b1, b2, b3;
    matrix bone = 0;
    matrix cedit = 0;
    matrix nedit = 0;
    matrix cS, cR, cT, nS, nR, nT;
    float4 cQ, nQ;

    float indices[4] = { input.BlendIndices.x, input.BlendIndices.y, input.BlendIndices.z, input.BlendIndices.w };
    float weights[4] = { input.BlendWeights.x, input.BlendWeights.y, input.BlendWeights.z, input.BlendWeights.w };
   
    uint clip[2];
    uint currFrame[2];
    uint nextFrame[2];
    float time[2];

    clip[0] = Tweenframes[input.InstID].Curr.Clip;
    currFrame[0] = Tweenframes[input.InstID].Curr.CurrFrame;
    nextFrame[0] = Tweenframes[input.InstID].Curr.NextFrame;
    time[0] = Tweenframes[input.InstID].Curr.Time;

    clip[1] = Tweenframes[input.InstID].Next.Clip;
    currFrame[1] = Tweenframes[input.InstID].Next.CurrFrame;
    nextFrame[1] = Tweenframes[input.InstID].Next.NextFrame;
    time[1] = Tweenframes[input.InstID].Next.Time;



    [unroll(4)]
    for (int i = 0; i < 4; i++)
    {
        cS = cR = cT = nS = nR = nT = 0;
        
        c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currFrame[0], clip[0], 0));
        c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currFrame[0], clip[0], 0));
        c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currFrame[0], clip[0], 0));
        c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currFrame[0], clip[0], 0));
        curr = matrix(c0, c1, c2, c3);

        n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], clip[0], 0));
        n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], clip[0], 0));
        n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], clip[0], 0));
        n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], clip[0], 0));
        next = matrix(n0, n1, n2, n3);
        
        currAnim = lerp(curr, next, time[0]);
               
        /* 애니메이션 변화 부분 */        
        b0 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 0, clip[0], 0));
        b1 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 1, clip[0], 0));
        b2 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 2, clip[0], 0));
        b3 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 3, clip[0], 0));
        
        cedit = matrix(b0, b1, b2, b3);
        
        [flatten]
        if (clip[1] >= 0)
        {
            cS = cR = cT = nS = nR = nT = 0;
            
            c0 = TransformsMap.Load(int4(indices[i] * 4 + 0, currFrame[1], clip[1], 0));
            c1 = TransformsMap.Load(int4(indices[i] * 4 + 1, currFrame[1], clip[1], 0));
            c2 = TransformsMap.Load(int4(indices[i] * 4 + 2, currFrame[1], clip[1], 0));
            c3 = TransformsMap.Load(int4(indices[i] * 4 + 3, currFrame[1], clip[1], 0));
            curr = matrix(c0, c1, c2, c3);

            n0 = TransformsMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], clip[1], 0));
            n1 = TransformsMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], clip[1], 0));
            n2 = TransformsMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], clip[1], 0));
            n3 = TransformsMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], clip[1], 0));
            next = matrix(n0, n1, n2, n3);
           
            nextAnim = lerp(curr, next, time[1]);
            
            /* 애니메이션 변화 부분 */
            b0 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 0, clip[1], 0));
            b1 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 1, clip[1], 0));
            b2 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 2, clip[1], 0));
            b3 = AnimEditTransformMap.Load(int3(indices[i] * 4 + 3, clip[1], 0));
        
            nedit = matrix(b0, b1, b2, b3);
            
        }
        
        
        //클립간 보간
        {
            //DivideMat(cS, cQ, cT, currAnim);
            //MatrixDecompose(cS, cQ, cT, currAnim);
            MatrixDecompose(cS, cQ, cT, currAnim);
            MatrixDecompose(nS, nQ, nT, nextAnim);
            cS = lerp(cS, nS, Tweenframes[input.InstID].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[input.InstID].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[input.InstID].TweenTime);
            
            currAnim = mul(cS, cR);
            currAnim = mul(currAnim, cT);
        
            
            MatrixDecompose(cS, cQ, cT, cedit);
            MatrixDecompose(nS, nQ, nT, nedit);
            cS = lerp(cS, nS, Tweenframes[input.InstID].TweenTime);
            cQ = normalize(lerp(cQ, nQ, Tweenframes[input.InstID].TweenTime));
            cR = QuattoMat(cQ);
            cT = lerp(cT, nT, Tweenframes[input.InstID].TweenTime);
            
            cedit = mul(cS, cR);
            cedit = mul(cedit, cT);
            currAnim = mul(currAnim, cedit);            
        }
        
        
        
        /* 본의 world */
        b0 = BoneTransformsMap.Load(int2(indices[i] * 4 + 0, 0));
        b1 = BoneTransformsMap.Load(int2(indices[i] * 4 + 1, 0));
        b2 = BoneTransformsMap.Load(int2(indices[i] * 4 + 2, 0));
        b3 = BoneTransformsMap.Load(int2(indices[i] * 4 + 3, 0));
        bone = matrix(b0, b1, b2, b3);
        currAnim = mul(bone,currAnim);
        
        transform += mul(weights[i], currAnim);
    }

    //world = transform;
    world = mul(transform, input.Transform);
}


MeshOutput VS_Animation(VertexModel input)
{
    MeshOutput output;

    matrix mat;
    SetAnimationWorld(World, input);
    //World = mul(mat, input.Transform);
    VS_GENERATE
        
    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);
    
    output.ID = input.InstID;
    output.Clip = 0;

    return output;
}

MeshOutput VS_Animation_GS(VertexModel input)
{
    MeshOutput output;
    SetAnimationWorld(World, input);

    VS_GENERATE
    output.wvpPosition = 0;

    output.sPosition = WorldPosition(input.Position);
    output.sPosition = mul(output.sPosition, ShadowView);
    output.sPosition = mul(output.sPosition, ShadowProjection);

    output.Clip = 0;
    return output;
}


DepthOutput VS_Depth_Animation(VertexModel input)
{
    DepthOutput output;

    SetAnimationWorld(World, input);
    output.Position = WorldPosition(input.Position);
    output.Position = mul(output.Position, ShadowView);
    output.Position = mul(output.Position, ShadowProjection);
    output.sPosition = output.Position;   

    return output;
}