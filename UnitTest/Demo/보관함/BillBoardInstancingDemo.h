#pragma once
#define MAX_INSTANCE 4096 //배열이 허용되는 최대 갯수

#include "Systems\IExecute.h"

class BillBoardInstancingDemo :public IExecute
{
private:
	// IExecute을(를) 통해 상속됨
	virtual void Initialize() override;
	virtual void Destroy() override;
	virtual void Update() override;
	virtual void Render() override;
	virtual void Ready() override {};
	virtual void PreRender() override {};
	virtual void PostRender() override {};
	virtual void ResizeScreen() override {};

private:
	void OpenFileDialog(UINT type, const wstring& filePath = L"");
	void OpenTxtDialog(const wstring& filePath = L"");

	void UpdateSplatting();

	wstring filePaths[4];

private:
	void CreateBills();
	void DeleteBills();
	void ReArrangeBills();
	//저장
	void SaveToXml(wstring savePath);
	//불러오기
	void LoadFromXml(wstring file = L"");
private:
	UINT meshPass = 0;
	UINT modelPass = 0;

	Shader* terrainShader;
	class Projector* projector;
	class Terrain* terrain;

	UINT treeIndex = 0;
	Shader * shader;
	class BillBoard* bill;
	//인스턴스 넣을 벡터
	vector<BillBoard*> bills;
	vector<Texture*> textureImages;		//콤보박스용

	class Sky* sky;

	Material* floor;
	MeshGrid* grid;

	int type = -1;
};
