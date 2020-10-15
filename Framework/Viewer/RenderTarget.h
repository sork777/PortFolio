#pragma once

/* 랜더링 한걸 저장 해줄 녀석 */
class RenderTarget
{
public:
	RenderTarget(UINT width=0, UINT height=0, DXGI_FORMAT format=DXGI_FORMAT_R8G8B8A8_UNORM);
	~RenderTarget();
	
	void Initialize();
	
	ID3D11RenderTargetView* RTV() { return rtv; }
	ID3D11ShaderResourceView* SRV() { return srv; }

	void SaveTexture(wstring file);
	void Set(ID3D11DepthStencilView* dsv,const Color& color= Color(0, 0, 0, 1));
	static void Sets(vector<RenderTarget*>& rtvs, ID3D11DepthStencilView* dsv);
private:
	UINT width, height;
	DXGI_FORMAT format;

	ID3D11Texture2D* backBuffer;
	ID3D11RenderTargetView* rtv;
	ID3D11ShaderResourceView* srv;
};

