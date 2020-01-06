#pragma once
#define MAX_RADIAL_BLUR_COUNT 32

namespace PostEffects
{
	class RadialBlur : public Render2D
	{
	public:
		RadialBlur();
		~RadialBlur();

		void Update() override;
		void Render() override;

	private:
		struct Desc
		{
			Vector2 MapSize;
			UINT BlurCount = 8;
			
			float Radius = 1.0f;
			float Amount = 0.04f; //4%
			float Offset = 1e-5f;

			float Padding[2];
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;
	};
}