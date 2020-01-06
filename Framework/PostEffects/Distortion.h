#pragma once

namespace PostEffects
{
	class Distortion : public Render2D
	{
	public:
		Distortion();
		~Distortion();

		void Update() override;
		void Render() override;

	private:
		struct Desc
		{
			float Noise = 0.01f;
			int Seed = 1337;

			Vector2 Padding;
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;
	};
}