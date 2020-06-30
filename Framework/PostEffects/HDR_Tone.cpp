#include "Framework.h"
#include "HDR_Tone.h"



HDR_Tone::HDR_Tone()
{
	shader = SETSHADER(L"HW05_HDR.fx");

	Init();
}

HDR_Tone::HDR_Tone(Shader* shader)
	:shader(shader)
{
	
	Init();
}


HDR_Tone::~HDR_Tone()
{
	SafeRelease(oldDSV);
	SafeRelease(oldRTV);
	SafeRelease(ds1Dbuffer);
	SafeRelease(ds1Duav);
	SafeRelease(ds1Dsrv);

	SafeRelease(avgLumBuffer);
	SafeRelease(avgLumUAV);
	SafeRelease(avgLumSRV);
}

void HDR_Tone::Init()
{
	width = D3D::Width();
	height = D3D::Height();
	dsGroups = (UINT)ceil((float)(width * height / 16) / 1024.0f);

	renderTarget = new RenderTarget(width, height, DXGI_FORMAT_R16G16B16A16_FLOAT);

	DSBuffer = new ConstantBuffer(&dsDesc, sizeof(DownScaleDesc));
	FinalPassBuffer = new ConstantBuffer(&fpDesc, sizeof(FinalPassDesc));

	sDSBuffer = shader->AsConstantBuffer("CB_DownScale");
	sFinalPassBuffer = shader->AsConstantBuffer("CB_FinalPass");
	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate down scaled luminance buffer
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	bufferDesc.StructureByteStride = sizeof(float);
	bufferDesc.ByteWidth = dsGroups * bufferDesc.StructureByteStride;
	bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &ds1Dbuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC DescUAV;
	ZeroMemory(&DescUAV, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	DescUAV.Format = DXGI_FORMAT_UNKNOWN;
	DescUAV.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	DescUAV.Buffer.NumElements = dsGroups;
	D3D::GetDevice()->CreateUnorderedAccessView(ds1Dbuffer, &DescUAV, &ds1Duav);


	//////////////////////////////////////////////////////////////////////////////////////////////////////
	// Allocate average luminance buffer
	D3D11_SHADER_RESOURCE_VIEW_DESC dsrvd;
	ZeroMemory(&dsrvd, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	dsrvd.Format = DXGI_FORMAT_UNKNOWN;
	dsrvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	dsrvd.Buffer.NumElements = dsGroups;
	D3D::GetDevice()->CreateShaderResourceView(ds1Dbuffer, &dsrvd, &ds1Dsrv);
	//////////////////////////////////////////////////////////////////////////////////////////////////////

	bufferDesc.ByteWidth = sizeof(float);
	D3D::GetDevice()->CreateBuffer(&bufferDesc, NULL, &avgLumBuffer);

	DescUAV.Buffer.NumElements = 1;
	D3D::GetDevice()->CreateUnorderedAccessView(avgLumBuffer, &DescUAV, &avgLumUAV);

	dsrvd.Buffer.NumElements = 1;
	D3D::GetDevice()->CreateShaderResourceView(avgLumBuffer, &dsrvd, &avgLumSRV);


}

void HDR_Tone::PostProcessing(ID3D11DepthStencilView * dsv)
{
	// Down scale the HDR image
	ID3D11RenderTargetView* rt[1] = { NULL };
	D3D::GetDC()->OMSetRenderTargets(1, rt, NULL);
	DownScale(renderTarget->SRV());
	// Do the final pass
	rt[0] = oldRTV;
	D3D::GetDC()->OMSetRenderTargets(1, rt, NULL);
	FinalPass(renderTarget->SRV());

	D3D::GetDC()->OMSetRenderTargets(1, rt, dsv);
}

void HDR_Tone::Set(ID3D11DepthStencilView * dsv)
{

	D3D::GetDC()->OMGetRenderTargets(1, &oldRTV, NULL);
	ID3D11RenderTargetView* rt[1] = { renderTarget->RTV() };
	D3D::GetDC()->OMSetRenderTargets(1, rt, dsv);
	
}

void HDR_Tone::DownScale(ID3D11ShaderResourceView * HDRSRV)
{
	//1st pass
	shader->AsUAV("AverageLum")->SetUnorderedAccessView(ds1Duav);
	shader->AsSRV("HDRTex")->SetResource(HDRSRV);

	dsDesc.Width = width / 4;
	dsDesc.Height = height / 4;
	dsDesc.TotalPixels = dsDesc.Width*dsDesc.Height;
	dsDesc.GroupSize = dsGroups;

	DSBuffer->Apply();
	sDSBuffer->SetConstantBuffer(DSBuffer->Buffer());

	shader->Dispatch(tech, dsPass, dsGroups, 1, 1);


	//2nd pass
	shader->AsUAV("AverageLum")->SetUnorderedAccessView(avgLumUAV);
	shader->AsSRV("HDRTex")->SetResource(ds1Dsrv);

	DSBuffer->Apply();
	sDSBuffer->SetConstantBuffer(DSBuffer->Buffer());

	shader->Dispatch(tech, avgPass, 1, 1, 1);
}

void HDR_Tone::FinalPass(ID3D11ShaderResourceView * HDRSRV)
{
	shader->AsSRV("HDRTex")->SetResource(HDRSRV);
	shader->AsSRV("AvgLum")->SetResource(avgLumSRV);

	fpDesc.MiddleGray = middleGray;
	fpDesc.LuminanceWhiteSqr = white;
	fpDesc.LuminanceWhiteSqr *= fpDesc.MiddleGray;
	fpDesc.LuminanceWhiteSqr *= fpDesc.LuminanceWhiteSqr;

	FinalPassBuffer->Apply();
	sFinalPassBuffer->SetConstantBuffer(FinalPassBuffer->Buffer());

	D3D::GetDC()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(tech, finalPass, 4);
}
