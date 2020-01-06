#pragma once

namespace PostEffects
{
	class Wiggle : public Render2D
	{
	public:
		Wiggle();
		~Wiggle();

		void Update() override;
		void Render() override;

	private:
		struct Desc
		{
			Vector2 Offset = Vector2(10, 10);
			Vector2 Amount = Vector2(0.01f, 0.01f);
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;
	};
}