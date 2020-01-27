#pragma once

struct DirectoryNode
{
	wstring DirectoryPath;
	wstring DirectoryName;
	vector< DirectoryNode> Children;
};
class Path
{
private:
	static vector<string> TextureFormats;
	static vector<string> AudioFormats;
	static vector<string> RawModelDataFormats;
	static vector<string> MeshFormats;
	static vector<string> RawModelFormats;
	static vector<string> ConvertedModelFormats;
	static vector<string> ConvertedAnimationFormats;
	static vector<string> MapFormats;

public:
	static bool IsSupportTextureFile(string path);
	static bool IsSupportTextureFile(wstring path);

	static bool IsSupportAudioFile(string path);
	static bool IsSupportAudioFile(wstring path);

	static bool IsSupportMeshFile(string path);
	static bool IsSupportMeshFile(wstring path);

	static bool IsRawModelFile(string path);
	static bool IsRawModelFile(wstring path);

	static bool IsConvertedModelFile(string path);
	static bool IsConvertedModelFile(wstring path);

	static bool IsConvertedAnimationFile(string path);
	static bool IsConvertedAnimationFile(wstring path);

	static bool IsSupportMapFile(string path);
	static bool IsSupportMapFile(wstring path);

public:
	static void GetDirectoryHierarchy(string path, DirectoryNode* parent);
	static void GetDirectoryHierarchy(wstring path, DirectoryNode* parent);

	static bool IsDirectory(string path);
	static bool IsDirectory(wstring path);
public:
	static bool ExistFile(string path);
	static bool ExistFile(wstring path);

	static bool ExistDirectory(string path);
	static bool ExistDirectory(wstring path);

	static string Combine(string path1, string path2);
	static wstring Combine(wstring path1, wstring path2);

	static string Combine(vector<string> paths);
	static wstring Combine(vector<wstring> paths);

	static string GetDirectoryName(string path);
	static wstring GetDirectoryName(wstring path);

	static string GetDirectDirectoryName(string path);
	static wstring GetDirectDirectoryName(wstring path);

	static string GetExtension(string path);
	static wstring GetExtension(wstring path);

	static string GetFileName(string path);
	static wstring GetFileName(wstring path);

	static string GetFileNameWithoutExtension(string path);
	static wstring GetFileNameWithoutExtension(wstring path);

	static string GetFilePathWithoutExtension(string path);
	static wstring GetFilePathWithoutExtension(wstring path);

public:
	const static WCHAR* ImageFilter;
	const static WCHAR* BinModelFilter;
	const static WCHAR* FbxModelFilter;
	const static WCHAR* ShaderFilter;
	const static WCHAR* XmlFilter;
	const static WCHAR* TextFilter;

	static void OpenFileDialog(wstring file, const WCHAR* filter, wstring folder, function<void(wstring)> func, HWND hwnd = NULL);
	static void SaveFileDialog(wstring file, const WCHAR* filter, wstring folder, function<void(wstring)> func, HWND hwnd = NULL);

public:
	static void GetFiles(vector<string>* files, string path, string filter, bool bFindSubFolder, bool bFindDirectSubFolder);
	static void GetFiles(vector<wstring>* files, wstring path, wstring filter, bool bFindSubFolder, bool bFindDirectSubFolder);

	static bool IsRelativePath(string path);
	static bool IsRelativePath(wstring path);

	static void CreateFolder(string path);
	static void CreateFolder(wstring path);

	static void CreateFolders(string path);
	static void CreateFolders(wstring path);

	static bool DeleteFolder(const string& path);
	static bool DeleteFolder(const wstring& path);

	static bool Delete_File(const string& path);
	static bool Delete_File(const wstring& path);

	static bool Copy_File(const string& src, const string& dst);
	static bool Copy_File(const wstring& src, const wstring& dst);	
};