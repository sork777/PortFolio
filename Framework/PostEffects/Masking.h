#pragma once

namespace PostEffects
{
	class Masking : public Render2D
	{
	public:
		Masking();
		~Masking();

		void Update() override;
		void Render() override;

	private:		
	};
}