#pragma once
#define MAX_BLOOM_COUNT 63

namespace PostEffects
{
	class Bloom : public Render2D
	{
	public:
		Bloom();
		~Bloom();

		void Update() override;
		void Render() override;

		void Map(
			ID3D11ShaderResourceView* l,
			ID3D11ShaderResourceView* x,
			ID3D11ShaderResourceView* y
		);
	private:
		struct BlurDesc
		{
			Vector2 Offset = Vector2(0, 0);
			float Weight = 0.0f;
			float Padding;
		};
		struct Desc
		{
			Vector2 MapSize;
			UINT BloomCount = 8;
			float Threshold = 0.25f;
			float Intensity = 1.15f;

			float Padding[3];

			BlurDesc BlurX[MAX_BLOOM_COUNT];
			BlurDesc BlurY[MAX_BLOOM_COUNT];
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;

		ID3DX11EffectShaderResourceVariable* sLuminosity;
		ID3DX11EffectShaderResourceVariable* sBlurX;
		ID3DX11EffectShaderResourceVariable* sBlurY;
	};
}