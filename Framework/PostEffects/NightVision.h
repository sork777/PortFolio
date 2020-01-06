#pragma once

namespace PostEffects
{
	class NightVision : public Render2D
	{
	public:
		NightVision();
		~NightVision();

		void Update() override;
		void Render() override;

	private:
		struct Desc
		{
			D3DXCOLOR Color = D3DXCOLOR(0.247f,0.698f,0.0f,1.0f);
			Vector3 Mono = Vector3(0.299f, 0.587f, 0.114f);
			float Weight = 0.1f;
			Vector2 MapSize;
			
			float Brightness =30.0f;
			float Padding;
		} desc;

		ConstantBuffer* buffer;
		ID3DX11EffectConstantBuffer* sBuffer;

		Texture* texture;
	};
}