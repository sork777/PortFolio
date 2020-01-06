#pragma once
#define MAX_INSTANCE 4096 //�迭�� ���Ǵ� �ִ� ����

#include "Systems\IExecute.h"

class BillBoardInstancingDemo :public IExecute
{
private:
	// IExecute��(��) ���� ��ӵ�
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
	//����
	void SaveToXml(wstring savePath);
	//�ҷ�����
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
	//�ν��Ͻ� ���� ����
	vector<BillBoard*> bills;
	vector<Texture*> textureImages;		//�޺��ڽ���

	class Sky* sky;

	Material* floor;
	MeshGrid* grid;

	int type = -1;
};
