#pragma once

class DepthStencil
{
public:
	DepthStencil(UINT width = 0, UINT height = 0, bool bUseStencil = false);
	~DepthStencil();

	ID3D11ShaderResourceView* SRV() { return srv; }
	void SaveTexture(wstring saveFile);

	ID3D11DepthStencilView* DSV() { return dsv; }
	ID3D11DepthStencilView* OnlyDSV() { return onlydsv; }

private:
	bool bUseStencil;
	UINT width, height;

	ID3D11Texture2D* backBuffer;
	ID3D11DepthStencilView* dsv;
	ID3D11DepthStencilView* onlydsv;
	ID3D11ShaderResourceView* srv;
};