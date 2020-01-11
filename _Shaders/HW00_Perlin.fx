//-----------------------------------------------------------------------------------------
// Perlin Noise
//-----------------------------------------------------------------------------------------
struct PerlinDesc
{
    float4 TopColor;
    float4 BottomColor;
    
    float2 Res;
    int Octave;
    float Persistence;
    float GridSize;
    int Seed;
    int SmoothedN;
};

cbuffer CB_Perlin
{
    PerlinDesc perlin;
};

RWTexture2D<float4> PerlinTex;

int Permute[512] =
{
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
		8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
		35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
		134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
		55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
		18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
		250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
		189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
		43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246,
		97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
		107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180,
    151, 160, 137, 91, 90, 15, 131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142,
		8, 99, 37, 240, 21, 10, 23, 190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117,
		35, 11, 32, 57, 177, 33, 88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71,
		134, 139, 48, 27, 166, 77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41,
		55, 46, 245, 40, 244, 102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89,
		18, 169, 200, 196, 135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226,
		250, 124, 123, 5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182,
		189, 28, 42, 223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167,
		43, 172, 9, 129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246,
		97, 228, 251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239,
		107, 49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
		138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180
};
float Smoothstep1(float x)
{
    return x * x * (3 - 2 * x);
}

float Smoothstep2(float x)
{
    return x * x * x * (x * (x * 6 - 15) + 10);
}

float grad(int hash, float x, float y, float z)
{
    int h = hash & 15;
	// Convert lower 4 bits of hash into 12 gradient directions
    float u = h < 8 ? x : y,
		v = h < 4 ? y : h == 12 || h == 14 ? x : z;
    return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}
float Lerp(float t, float a, float b)
{
    return a + t * (b - a);
}
float Noise(float x, float y, float z, bool bStep1)
{
    int X = (int) floor(x) & 255;
    int Y = (int) floor(y) & 255;
    int Z = (int) floor(z) & 255;

	// Find relative x, y,z of point in cube
    x -= floor(x);
    y -= floor(y);
    z -= floor(z);

	// Compute fade curves for each of x, y, z
    float u = bStep1 ? Smoothstep1(x) : Smoothstep2(x);
    float v = bStep1 ? Smoothstep1(y) : Smoothstep2(y);
    float w = bStep1 ? Smoothstep1(z) : Smoothstep2(z);

    int size = perlin.Res.x;
	// Hash coordinates of the 8 cube corners
    X = (X + perlin.Seed) % size;
    Y = (Y + perlin.Seed) % size;
    Z = (Z + perlin.Seed) % size;
    
    int A = Permute[X] + Y;
    A = (A + perlin.Seed) % size;
    int AA = Permute[A] + Z;
    AA = (AA + perlin.Seed) % size;
    int AB = Permute[A + 1] + Z;
    AB = (AB + perlin.Seed) % size;
    int B = Permute[X + 1] + Y;
    B = (B + perlin.Seed) % size;
    int BA = Permute[B] + Z;
    BA = (BA + perlin.Seed) % size;
    int BB = Permute[B + 1] + Z;
    BB = (BB + perlin.Seed) % size;

	// Add blended results from 8 corners of cube
    float res = Lerp(w, Lerp(v, Lerp(u, grad(Permute[AA], x, y, z), grad(Permute[BA], x - 1, y, z)), Lerp(u, grad(Permute[AB], x, y - 1, z), grad(Permute[BB], x - 1, y - 1, z))), Lerp(v, Lerp(u, grad(Permute[AA + 1], x, y, z - 1), grad(Permute[BA + 1], x - 1, y, z - 1)), Lerp(u, grad(Permute[AB + 1], x, y - 1, z - 1), grad(Permute[BB + 1], x - 1, y - 1, z - 1))));
    return (res + 1.0) / 2.0;
}

[numthreads(1024, 1, 1)]
void PerlinNoiseCS(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    
    uint3 CurPixel = uint3(DispatchThreadID.x % perlin.Res.x, DispatchThreadID.x / perlin.Res.x, 0);
    float4 color = 0;
    float total=0;
    float x = float(CurPixel.x) / float(perlin.Res.x);
    float y = float(CurPixel.y) / float(perlin.Res.y);
    bool bUse1 = perlin.SmoothedN < 1 ? true : false;
    float p = perlin.Persistence;
    int n = perlin.Octave;
    float maxvalue = 0;
    //[unroll]
    for (int i = 0; i < n;i++)
    {
        float freq = pow(2, i)*perlin.GridSize;
        float amp = pow(p, i);
        total += Noise(x * freq, y * freq, 0.6 * freq, bUse1) * amp;
        maxvalue += amp;
    }
    total /= maxvalue;
    color = lerp(perlin.BottomColor, perlin.TopColor, total);    
    
    PerlinTex[CurPixel.xy] = color;
}

technique11 T0
{
    pass P0
    {
        SetVertexShader(NULL);
        SetPixelShader(NULL);
        SetComputeShader(CompileShader(cs_5_0, PerlinNoiseCS()));
    }
  
}