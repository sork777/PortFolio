#pragma once

class Converter
{
public:
	Converter();
	~Converter();
	void ReadFile(wstring file, wstring directory= L"../../_Assets/");

public:
	void ExportMaterial(wstring savePath, wstring directoryPath= L"../../_Textures/", bool bOverwrite = true);

private:
	void ReadMaterial();
	void WriteMaterial(wstring savePath, bool bOverwrite);
	string WriteTexture(string savePath, string file);

public:
	void ExportMesh(wstring savePath, wstring directoryPath= L"../../_Models/", bool bOverwrite = true);

private:
	void ReadBoneData(aiNode* node, int index, int parent);
	void ReadMeshData(aiNode* node, int bone);
	void ReadSkinData();
	void WriteMeshData(wstring savePath, bool bOverwrite);

public:
	void ClipList(vector<wstring>* list);
	void ExportAnimClip(UINT index, wstring savePath, wstring directoryPath = L"../../_Models/", bool bOverwrite = true);

private:
	struct asClip* ReadClipData(aiAnimation* animation);
	void ReadKeyframeData(struct asClip* clip, aiNode* node, vector<struct asClipNode>& aiNodeInfos);

	void WriteClipData(struct asClip* clip, wstring savePath, bool bOverwrite);


private:
	wstring file;

	Assimp::Importer* importer;
	const aiScene* scene;

	vector<struct asMaterial *> materials;
	vector<struct asBone *> bones;
	vector<struct asMesh *> meshes;
};