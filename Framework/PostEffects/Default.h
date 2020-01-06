#pragma once

namespace PostEffects
{
	class Default : public Render2D
	{
	public:
		Default();
		~Default();

		void Update() override;
		void Render() override;

	private:		
	};
}