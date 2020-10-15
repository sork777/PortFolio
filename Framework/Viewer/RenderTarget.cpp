#include "Framework.h"
#include "RenderTarget.h"

RenderTarget::RenderTarget(UINT width, UINT height, DXGI_FORMAT format)
	:format(format)
{
	this->width = (width < 1) ? (UINT)D3D::Width() : width;
	this->height = (height < 1) ? (UINT)D3D::Height() : height;

	Initialize();
}

RenderTarget::~RenderTarget()
{
	SafeRelease(backBuffer);
	SafeRelease(rtv);
	SafeRelease(srv);
}

void RenderTarget::Initialize()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	{
		ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
		textureDesc.Width = this->width;
		textureDesc.Height = this->height;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;

		Check(D3D::GetDevice()->CreateTexture2D(&textureDesc, NULL, &backBuffer));


		/* RTV */
		D3D11_RENDER_TARGET_VIEW_DESC rtvDesc;
		ZeroMemory(&rtvDesc, sizeof(D3D11_RENDER_TARGET_VIEW_DESC));
		rtvDesc.Format = format;
		rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtvDesc.Texture2D.MipSlice = 0;

		D3D::GetDevice()->CreateRenderTargetView(backBuffer, &rtvDesc, &rtv);

		/* SRV */
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		srvDesc.Format = format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = 1;

		D3D::GetDevice()->CreateShaderResourceView(backBuffer, &srvDesc, &srv);
	}
}

void RenderTarget::SaveTexture(wstring file)
{
	Check(D3DX11SaveTextureToFile(D3D::GetDC(), backBuffer, D3DX11_IFF_PNG, file.c_str()));
}

void RenderTarget::Set(ID3D11DepthStencilView * dsv, const Color& color)
{
	D3D::Get()->SetRenderTarget(rtv, dsv);
	D3D::Get()->Clear(color, rtv, dsv);
}

void RenderTarget::Sets(vector<RenderTarget*>& rtvs, ID3D11DepthStencilView * dsv)
{
	vector<ID3D11RenderTargetView*> views;
	for (UINT i = 0; i < rtvs.size(); i++)
		views.push_back(rtvs[i]->RTV());

	D3D::GetDC()->OMSetRenderTargets(views.size(), &views[0], dsv);
	
	for (UINT i = 0; i < rtvs.size(); i++)
		D3D::Get()->Clear(Color(0, 0, 0, 0),views[i],dsv);
}
