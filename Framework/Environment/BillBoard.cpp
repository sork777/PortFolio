#include "Framework.h"
#include "BillBoard.h"

BillBoard::BillBoard()
	: Renderer(L"025_Geometry_BillBoard.fx")
	, fixedY(true), bChangeCount(false)
	, drawCount(0)
	, windParm(0.0f,0.0f), billScale(1.0f,1.0f)
	, time(0.0f)
{
	/* 포인트 리스트인것 잘보기 */
	//포인트라 인덱스도 없음
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	sSRV = shader->AsSRV("BillTex");
	shader->AsScalar("FixedY")->SetBool(fixedY);
	shader->AsVector("BillScale")->SetFloatVector(billScale);

}

BillBoard::~BillBoard()
{
	SafeRelease(sSRV);
	SafeRelease(billSrv);
	SafeDelete(billTex);
	

}

void BillBoard::Clear()
{
	vertices.clear();
	vertices.shrink_to_fit();
}

void BillBoard::Update()
{
	if (NULL == billTex) return;
	if (true == bChangeCount) {
		ResizeBuffer();
		bChangeCount = false;
	}
	Renderer::Update();
}

void BillBoard::Render()
{
	if (NULL == billTex) return;
	Renderer::Render();

	time += Time::Delta();
	Vector2 wind = windParm * abs(sinf(time))*0.5f;
	
	shader->AsVector("WindParm")->SetFloatVector(wind);
	shader->Draw(Tech(), Pass(), vertices.size());
}

void BillBoard::FixedY(const bool & val)
{
	fixedY = val;
	shader->AsScalar("FixedY")->SetBool(fixedY);
}

void BillBoard::AddInstance(Vector3 & position)
{
	if (drawCount + 1 >= MAX_BILL_INSTANCE)
		return;

	VertexScale vertex;
	vertex.Position = position;

	vertices.push_back(vertex);
	drawCount++;
	bChangeCount = true;
}

void BillBoard::DeleteInstance(const UINT&  index)
{
	if (index < drawCount);
	{	
		vertices.erase(vertices.begin() + index);
		drawCount--;
		bChangeCount = true;
	}
}
const bool& BillBoard::GetBillBoardPos(const UINT & index, OUT Vector3* position)
{
	if (index < drawCount)
	{
		*position = vertices[index].Position;
		return true;
	}
	else
		return false;
}
const bool& BillBoard::SetBillBoardPos(const UINT & index, const Vector3 & position)
{
	if (index < drawCount)
	{
		vertices[index].Position = position;
		return true;
	}
	else
		return false;
}

void BillBoard::SetBillBoardScale(const Vector2 & scale)
{
	billScale = scale;
	shader->AsVector("BillScale")->SetFloatVector(billScale);
}

//정점 위치 변하면 다시 해줘야
void BillBoard::ResizeBuffer()
{
	SafeDelete(vertexBuffer);
	if (vertices.size() > 0)
	{
		vertexBuffer = new VertexBuffer(&vertices[0], vertices.size(), sizeof(VertexScale));
	}
}

void BillBoard::TextureSelecter(const wstring & filePath)
{
	//TODO: 나중에 opendialog말고 Asset을 통해 선택하게 하기	
	if (filePath.length() < 1)
	{
		Path::OpenFileDialog(L"", Path::ImageFilter, L"", bind(&BillBoard::TextureSelecter, this, placeholders::_1));
	}
	else
	{
		SafeRelease(billSrv);
		SafeDelete(billTex);
		wstring TexFilePath = Path::GetFileName(filePath);
		wstring TexDirPath = Path::GetDirectoryName(filePath);
		billTex = new Texture(TexFilePath,TexDirPath);
		billSrv = billTex->SRV();
		sSRV->SetResource(billSrv);
	}
}

const bool& BillBoard::BillBoardButton()
{
	bool result =  ImGui::ImageButton(billSrv, ImVec2(50, 50));
	return result;
}
