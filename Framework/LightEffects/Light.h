#pragma once
#define MAX_PLIGHTS 500

namespace Lights
{
	
	struct PointLightInfo
	{
		Color color;
		Vector3 Position;
		float Range;
		float Specular;
	};

	struct SpotLightInfo
	{
		Color color;
		Vector3 Position;
		float Range;
		Vector3 Direction;
		float OuterAngle;
		float InnerAngle;
		float Specular;
	};

	struct CapsuleLightInfo
	{
		Color color;
		
		Vector3 Position;
		float Range;
		
		Vector3 Direction;
		float Length;
		float Specular;
	};

	class Light
	{
	public:
		Light(Shader* shader);
		~Light();

		void SetSize(UINT width = 0, UINT height = 0);
		void Pass(UINT val) { pass = val; }
		void Tech(UINT val) { tech = val; }

		virtual void Render()=0;
		virtual void LightController() = 0;
		
		virtual void SetIsDrawing(Vector3& objPos, float drawRad) {};

	protected:
		Shader* shader;

		UINT pass = 0;
		UINT tech = 0;

		UINT width, height;
		UINT lightIndex = 0;
		
		UINT lightCount = 0;
		ConstantBuffer* cbuffer;
		ID3DX11EffectConstantBuffer* sCbuffer;		
		ID3DX11EffectMatrixVariable* sShadowBuffer;

		D3D11_TEXTURE2D_DESC textureDesc;
		D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	};

	/////////////////////////////////////////////////////////////////////
	
	class PointLight:public Light
	{
	public:
		PointLight(Shader* shader);
		virtual ~PointLight();

		void Render() override;
		void LightController() override;

		void SetIsDrawing(Vector3& objPos, float drawRad) override;
		ID3D11ShaderResourceView* ShadowSRV(UINT index)  { return shadows[index].srv; }
		bool LightPreSet();

		void AddLight(PointLightInfo& light);

	private:
		struct LightInfo
		{
			Matrix LightProjection;
			Color color;

			Vector3 Position;
			float RangeReverse;

			Vector2 LightPerspectiveValues;
			float Specular;
			float padding;
		};

		struct LightDesc
		{
			LightInfo Lights;
		}pointlightDesc;

		struct ShadowInfo
		{
			Matrix ViewProjection[6];
			bool bShadow = true;
			char padding[3];

			ID3D11Texture2D* backBuffer;
			ID3D11DepthStencilView* dsv;
			ID3D11ShaderResourceView* srv;
		};

	private:
		vector<LightInfo> lights;
		vector<ShadowInfo> shadows;
	};

	////////////////////////////////////////////////////////////////////////////////
	// Spot Light //
	////////////////////////////////////////////////////////////////////////////////

	class SpotLight :public Light
	{
	public:
		SpotLight(Shader* shader);
		virtual ~SpotLight();

		void Render() override;
		void LightController() override;

		void SetIsDrawing(Vector3& objPos, float drawRad) override;

		ID3D11ShaderResourceView* ShadowSRV(UINT index) { return shadows[index].srv; }
		bool LightPreSet();

		void AddLight(SpotLightInfo& light);

	private:
		struct LightInfo
		{
			Color color;

			Vector3 Position;
			float RangeReverse;

			Vector3 Direction;
			float SpotCosOuterCone;

			Matrix LightProjection;
			
			float SpotCosConeAttRange;
			float SinAngle;
			float CosAngle;
			float Specular;
			
			float OuterAngle;
			float InnerAngle;			
			float padding[2];
		};

		struct LightDesc
		{
			LightInfo Lights;
		}spotlightDesc;

		struct ShadowInfo
		{
			Matrix ToShadowMap;
			bool bShadow = true;
			char padding[3];

			ID3D11Texture2D* backBuffer;
			ID3D11DepthStencilView* dsv;
			ID3D11ShaderResourceView* srv;
		};

	private:
		vector<LightInfo> lights;
		vector<ShadowInfo> shadows;
	};
	////////////////////////////////////////////////////////////////////////////////
	// Capsule Light //
	////////////////////////////////////////////////////////////////////////////////

	class CapsuleLight :public Light
	{
	public:
		CapsuleLight(Shader* shader);
		virtual ~CapsuleLight();

		void Render() override;
		void LightController() override;

		void AddLight(CapsuleLightInfo& light);

	private:
		struct LightInfo
		{
			Color color;

			Vector3 Position;
			float RangeRcp;

			Vector3 Direction;
			float Length;

			Matrix LightProjection;
			
			float HalfLen;
			float Range;
			float Specular;
			float padding;
		};

		struct LightDesc
		{
			LightInfo Lights;
		}capsulelightDesc;

	private:
		vector<LightInfo> lights;
	};



}