#include "Framework.h"
#include "FFTOceanSimulator.h"

// Generating gaussian random number with mean 0 and standard deviation 1.
float Gauss()
{
	float u1 = rand() / (float)RAND_MAX;
	float u2 = rand() / (float)RAND_MAX;
	if (u1 < 1e-6f)
		u1 = 1e-6f;
	return sqrtf(-2 * logf(u1)) * cosf(2 * D3DX_PI * u2);
}

// Phillips Spectrum
// K: normalized wave vector, W: wind direction, v: wind velocity, a: amplitude constant
float Phillips(Vector2 K, Vector2 W, float v, float a, float dir_depend)
{
	// largest possible wave from constant wind of velocity v
	float l = v * v / GRAV_ACCEL;
	// damp out waves with very small length w << l
	float w = l / 1000;

	float Ksqr = K.x * K.x + K.y * K.y;
	float Kcos = K.x * W.x + K.y * W.y;
	float phillips = a * expf(-1 / (l * l * Ksqr)) / (Ksqr * Ksqr * Ksqr) * (Kcos * Kcos);

	// filter out waves moving opposite to wind
	if (Kcos < 0)
		phillips *= dir_depend;

	// damp out waves with very small length w << l
	return phillips * expf(-Ksqr * w * w);
}

void createTextureAndViews(UINT width, UINT height, DXGI_FORMAT format,
	ID3D11Texture2D** ppTex, ID3D11ShaderResourceView** ppSRV, ID3D11RenderTargetView** ppRTV)
{
	// Create 2D texture
	D3D11_TEXTURE2D_DESC tex_desc;
	tex_desc.Width = width;
	tex_desc.Height = height;
	tex_desc.MipLevels = 0;
	tex_desc.ArraySize = 1;
	tex_desc.Format = format;
	tex_desc.SampleDesc.Count = 1;
	tex_desc.SampleDesc.Quality = 0;
	tex_desc.Usage = D3D11_USAGE_DEFAULT;
	tex_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	tex_desc.CPUAccessFlags = 0;
	tex_desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	D3D::GetDevice()->CreateTexture2D(&tex_desc, NULL, ppTex);
	assert(*ppTex);

	// Create shader resource view
	(*ppTex)->GetDesc(&tex_desc);
	if (ppSRV)
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
		srv_desc.Format = format;
		srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srv_desc.Texture2D.MipLevels = tex_desc.MipLevels;
		srv_desc.Texture2D.MostDetailedMip = 0;

		D3D::GetDevice()->CreateShaderResourceView(*ppTex, &srv_desc, ppSRV);
		assert(*ppSRV);
	}

	// Create render target view
	if (ppRTV)
	{
		D3D11_RENDER_TARGET_VIEW_DESC rtv_desc;
		rtv_desc.Format = format;
		rtv_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
		rtv_desc.Texture2D.MipSlice = 0;

		D3D::GetDevice()->CreateRenderTargetView(*ppTex, &rtv_desc, ppRTV);
		assert(*ppRTV);
	}
}


FFTOceanSimulator::FFTOceanSimulator(OceanParameter& params)
{
	fftPlan = new FFT();
	shader = new Shader(L"HW07_OceanSimulator.fxo");

	// Height map H(0)
	// 0일때의 높이맵 H(0)
	int height_map_size = (params.dmap_dim + 4) * (params.dmap_dim + 1);
	Vector2* h0_data = new Vector2[height_map_size * sizeof(Vector2)];
	float* omega_data = new float[height_map_size * sizeof(float)];
	initHeightMap(params, h0_data, omega_data);

	oceanParam = params;
	int hmap_dim = params.dmap_dim;
	int input_full_size = (hmap_dim + 4) * (hmap_dim + 1);
	// This value should be (hmap_dim / 2 + 1) * hmap_dim, but we use full sized buffer here for simplicity.
	int input_half_size = hmap_dim * hmap_dim;
	int output_size = hmap_dim * hmap_dim;

	// 0으로 채워진 데이터 버퍼
	char* zero_data = new char[3 * output_size * sizeof(float) * 2];
	memset(zero_data, 0, 3 * output_size * sizeof(float) * 2);
	//ZeroMemory(&zero_data, 3 * output_size * sizeof(float) * 2);

	// RW buffer allocations
	// H0
	UINT float2_stride = 2 * sizeof(float);
	Float2_H0 = new StructuredBuffer(h0_data, float2_stride, input_full_size);

	// Notice: The following 3 buffers should be half sized buffer because of conjugate symmetric input.
	// But, we use full sized buffers due to the CS4.0 restriction.

	// Put H(t), Dx(t) and Dy(t) into one buffer because CS4.0 allows only 1 UAV at a time
	Float2_Ht = new StructuredBuffer(zero_data, float2_stride, 3 * input_half_size);

	// omega
	Float_Omega = new StructuredBuffer(omega_data, sizeof(float), input_full_size);
	
	// Notice: The following 3 should be real number data. But here we use the complex numbers and C2C FFT
	// due to the CS4.0 restriction.
	// Put Dz, Dx and Dy into one buffer because CS4.0 allows only 1 UAV at a time
	Float_Dxyz = new StructuredBuffer(zero_data, float2_stride, 3 * output_size);

	//할당 데이터 삭제
	SafeDeleteArray(zero_data);
	SafeDeleteArray(h0_data);
	SafeDeleteArray(omega_data);

	// D3D11 Textures
	createTextureAndViews(hmap_dim, hmap_dim, DXGI_FORMAT_R32G32B32A32_FLOAT, &pDisplacementMap, &pDisplacementSRV, &pDisplacementRTV);
	createTextureAndViews(hmap_dim, hmap_dim, DXGI_FORMAT_R16G16B16A16_FLOAT, &pGradientMap, &pGradientSRV, &pGradientRTV);

	// Quad vertex buffer
	
	Vector4 quad_vertices[4];
	quad_vertices[0] = Vector4(-1, -1, 0, 1);
	quad_vertices[1] = Vector4(-1, 1, 0, 1);
	quad_vertices[2] = Vector4(1, -1, 0, 1);
	quad_vertices[3] = Vector4(1, 1, 0, 1);
	
	QuadVB = new VertexBuffer(&quad_vertices, 4, sizeof(Vector4));

	// Constant buffers
	UINT actual_dim = oceanParam.dmap_dim;
	UINT input_width = actual_dim + 4;
	// We use full sized data here. The value "output_width" should be actual_dim/2+1 though.
	UINT output_width = actual_dim;
	UINT output_height = actual_dim;
	UINT dtx_offset = actual_dim * actual_dim;
	UINT dty_offset = actual_dim * actual_dim * 2;
	immuteDesc = { actual_dim, input_width, output_width, output_height, dtx_offset, dty_offset };

	ImmutableCB = new ConstantBuffer(&immuteDesc, sizeof(ImmutableDesc));
	PerFrameCB = new ConstantBuffer(&changePFDesc, sizeof(changePFDesc));
	ImmutableCB->Apply();


	fftPlan->Create_Plan(3);
}

FFTOceanSimulator::~FFTOceanSimulator()
{
	SafeDelete(shader);
	SafeDelete(fftPlan);

	SafeDelete(Float2_H0);
	SafeDelete(Float_Omega);
	SafeDelete(Float2_Ht);
	SafeDelete(Float_Dxyz);

	SafeDelete(ImmutableCB);
	SafeDelete(PerFrameCB);

	SafeDelete(QuadVB);

	SafeRelease(pDisplacementMap);
	SafeRelease(pDisplacementSRV);
	SafeRelease(pDisplacementRTV);

	SafeRelease(pGradientMap);
	SafeRelease(pGradientSRV);
	SafeRelease(pGradientRTV);

}

void FFTOceanSimulator::UpdateDisplacementMap(float time)
{
	// ---------------------------- H(0) -> H(t), D(x, t), D(y, t) --------------------------------
	shader->AsSRV("InputH0")->SetResource(Float2_H0->SRV());
	shader->AsSRV("InputOmega")->SetResource(Float_Omega->SRV());
	shader->AsUAV("OutputHt")->SetUnorderedAccessView(Float2_Ht->UAV());

	// Constant Buffer Update
	changePFDesc.Time = time * oceanParam.time_scale;
	changePFDesc.ChoppyScale = oceanParam.choppy_scale;
	changePFDesc.GridLen = oceanParam.dmap_dim/ oceanParam.patch_length;
	
	// Constants
	shader->AsConstantBuffer("CB_Immutable")->SetConstantBuffer(ImmutableCB->Buffer());
	PerFrameCB->Apply();
	shader->AsConstantBuffer("CB_ChangePerFrame")->SetConstantBuffer(PerFrameCB->Buffer());

	// Run the CS
	UINT group_count_x = (oceanParam.dmap_dim + BLOCK_SIZE_X - 1) / BLOCK_SIZE_X;
	UINT group_count_y = (oceanParam.dmap_dim + BLOCK_SIZE_Y - 1) / BLOCK_SIZE_Y;
	shader->Dispatch(0, CSPass,group_count_x, group_count_y, 1);

	//static bool bFile = false;
	//if (ImGui::Button("save"))
	//	bFile = true;
	//if(bFile)
	//{
	//	UINT size = Float2_Ht->OutputByteWidth() / sizeof(Vector2);
	//	Vector2* output = new Vector2[size];
	//	Float2_Ht->Copy(output, sizeof(Vector2)* size);

	//	string str = "TestFFTOceanSimulator_Ht.csv";
	//	FILE* file;
	//	fopen_s(&file, str.c_str(), "w");
	//	for (UINT i = 0; i < size; i++)
	//	{
	//		Vector2 temp = output[i];

	//		fprintf(file,
	//			"%d,%f,%f\n",
	//			i,
	//			temp.x, temp.y
	//		);
	//	}
	//	fclose(file);

	//	SafeDeleteArray(output);
	//}

	// ------------------------------------ Perform FFT -------------------------------------------
	fftPlan->fft_512x512_c2c(Float_Dxyz->UAV(), Float_Dxyz->SRV(), Float2_Ht->SRV());
	/*if (bFile)
	{
		UINT size = Float_Dxyz->OutputByteWidth() / sizeof(Vector2);
		Vector2* output = new Vector2[size];
		Float_Dxyz->Copy(output, sizeof(Vector2)* size);

		string str = "TestFFTOceanSimulator_Dxyz.csv";
		FILE* file;
		fopen_s(&file, str.c_str(), "w");
		for (UINT i = 0; i < size; i++)
		{
			Vector2 temp = output[i];

			fprintf(file,
				"%d,%f,%f\n",
				i,
				temp.x, temp.y
			);
		}
		fclose(file);

		SafeDeleteArray(output);
		bFile = false;
	}
*/
	// --------------------------------- Wrap Dx, Dy and Dz ---------------------------------------
	// Push RT
	ID3D11RenderTargetView* old_target;
	ID3D11DepthStencilView* old_depth;
	D3D::GetDC()->OMGetRenderTargets(1, &old_target, &old_depth);
	D3D11_VIEWPORT old_viewport;
	UINT num_viewport = 1;
	D3D::GetDC()->RSGetViewports(&num_viewport, &old_viewport);
	
	D3D11_VIEWPORT new_vp = { 0, 0, (float)oceanParam.dmap_dim, (float)oceanParam.dmap_dim, 0.0f, 1.0f };
	D3D::GetDC()->RSSetViewports(1, &new_vp);
	// Set RT
	D3D::GetDC()->OMSetRenderTargets(1, &pDisplacementRTV, NULL);

	// Constants
	shader->AsConstantBuffer("CB_Immutable")->SetConstantBuffer(ImmutableCB->Buffer());
	shader->AsConstantBuffer("CB_ChangePerFrame")->SetConstantBuffer(PerFrameCB->Buffer());

	// Buffer resources
	shader->AsSRV("InputDxyz")->SetResource(Float_Dxyz->SRV());
	// IA setup
	QuadVB->Render();
	D3D::GetDC()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	shader->Draw(0, disPass,4);



	// ----------------------------------- Generate Normal ----------------------------------------
	// Set RT
	D3D::GetDC()->OMSetRenderTargets(1, &pGradientRTV, NULL);

	// Texture resource and sampler
	shader->AsSRV("samplerDisplacementMap")->SetResource(pDisplacementSRV);
	// Perform draw call
	shader->Draw(0, gradPass, 4);
	
	// Pop RT
	D3D::GetDC()->RSSetViewports(1, &old_viewport);
	D3D::GetDC()->OMSetRenderTargets(1, &old_target, old_depth);
	SafeRelease(old_target);
	SafeRelease(old_depth);

	D3D::GetDC()->GenerateMips(pGradientSRV);
}

void FFTOceanSimulator::initHeightMap(OceanParameter & params, Vector2 * out_h0, float * out_omega)
{
	int i, j;
	Vector2 K, Kn;

	Vector2 wind_dir;
	D3DXVec2Normalize(&wind_dir, &params.wind_dir);
	float a = params.wave_amplitude * 1e-7f;	// It is too small. We must scale it for editing.
	float v = params.wind_speed;
	float dir_depend = params.wind_dependency;

	int height_map_dim = params.dmap_dim;
	float patch_length = params.patch_length;

	// initialize random generator.
	srand(0);

	for (i = 0; i <= height_map_dim; i++)
	{
		// K is wave-vector, range [-|DX/W, |DX/W], [-|DY/H, |DY/H]
		K.y = (-height_map_dim / 2.0f + i) * (2 * D3DX_PI / patch_length);

		for (j = 0; j <= height_map_dim; j++)
		{
			K.x = (-height_map_dim / 2.0f + j) * (2 * D3DX_PI / patch_length);

			float phil = (K.x == 0 && K.y == 0) ? 0 : sqrtf(Phillips(K, wind_dir, v, a, dir_depend));

			out_h0[i * (height_map_dim + 4) + j].x = float(phil * Gauss() * HALF_SQRT_2);
			out_h0[i * (height_map_dim + 4) + j].y = float(phil * Gauss() * HALF_SQRT_2);

			// The angular frequency is following the dispersion relation:
			//            out_omega^2 = g*k
			// The equation of Gerstner wave:
			//            x = x0 - K/k * A * sin(dot(K, x0) - sqrt(g * k) * t), x is a 2D vector.
			//            z = A * cos(dot(K, x0) - sqrt(g * k) * t)
			// Gerstner wave shows that a point on a simple sinusoid wave is doing a uniform circular
			// motion with the center (x0, y0, z0), radius A, and the circular plane is parallel to
			// vector K.
			out_omega[i * (height_map_dim + 4) + j] = sqrtf(GRAV_ACCEL * sqrtf(K.x * K.x + K.y * K.y));
		}
	}
}

