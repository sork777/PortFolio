#pragma once

class Mirror : public Renderer
{
private:
	typedef VertexTextureNormalTangent mirrorV;
public:
	Mirror(Shader* shader, float w, float h, float d);
	~Mirror();

	void Update() override;
	void Render() override;
	void MirrorRender();
private:
	struct Desc
	{
		Matrix ReflectionMatrix;
		Color RefractionColor = Color(0, 0.3f, 1.0f, 1.0f);
		float Shininess=200.0f;

		float Padding[3];
	}desc;
private:
	float h, w, d;
	mirrorV* vertices;
	UINT* indices;

	ConstantBuffer* buffer;

	Vector2 mapScale;
	Texture* mirrorImg;


	DepthStencil* depthStencil;
	Viewport* viewport;

	//ID3DX11EffectShaderResourceVariable* sReflection;
	ID3DX11EffectVectorVariable* sClipPlane;

};

