struct ResultObject
{
    float4 ColorID;
    uint instNum;
};

RWStructuredBuffer<ResultObject> Output;
Texture2D ColorIDTexture;
Texture2D InstTexture;

struct PickerInfo
{
    float2 Res;
    float2 MousePos;
};
cbuffer CB_Picker
{
    PickerInfo Pinfo;
};


#include "000_Math.fx"

///////////////////////////////////////////////////////////////////////////////

[numthreads(1024, 1, 1)]
void CS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    //uint3 CurPixel = uint3(DispatchThreadID.x % Pinfo.Res.x, DispatchThreadID.x / Pinfo.Res.x, 0);
    uint3 pickupPix = uint3(Pinfo.MousePos, 0);
    
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