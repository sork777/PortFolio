#pragma once

namespace PostEffects
{
	class Vignette : public Render2D
	{
	public:
		Vignette();
		~Vignette();

		void Update() override;
		void Render() override;

	private:
		struct Desc
		{
			Vector2 MapSize;
			Vector2 Scale = Vector2(1, 1);

			float Vignette = 1;
			float Padding[3];
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;
	};
}