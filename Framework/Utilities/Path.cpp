#include "Framework.h"
#include "Path.h"
#include <string.h>
#include <shlwapi.h>
#include "String.h"
#include <experimental\filesystem>

using namespace std::experimental::filesystem;
#pragma comment(lib, "shlwapi.lib")

#pragma region 지원파일확장자

vector<string> Path::TextureFormats
{
	"jpg",
	"png",
	"bmp",
	"tga",
	"dds",
};
vector<string> Path::AudioFormats
{
	"mp3",
	"wav",	
};
vector<string> Path::MeshFormats
{
	"mesh",
};
vector<string> Path::RawModelFormats
{
	"fbx",
};
vector<string> Path::ConvertedModelFormats
{
	"material",
};
vector<string> Path::ConvertedAnimationFormats
{
	"fbx",
	"clip",
	"anim",
};
vector<string> Path::MapFormats
{
	"alphamap",
	"heightmap",
	"map",
};

#pragma endregion

#pragma region 지원확인영역

bool Path::IsSupportTextureFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : TextureFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsSupportTextureFile(wstring path)
{
	return IsSupportTextureFile(String::ToString(path));
}

bool Path::IsSupportAudioFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : AudioFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsSupportAudioFile(wstring path)
{
	return IsSupportAudioFile(String::ToString(path));
}

bool Path::IsSupportMeshFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : MeshFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsSupportMeshFile(wstring path)
{
	return IsSupportMeshFile(String::ToString(path));
}

bool Path::IsRawModelFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : RawModelFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsRawModelFile(wstring path)
{
	return IsRawModelFile(String::ToString(path));
}

bool Path::IsConvertedModelFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : ConvertedModelFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsConvertedModelFile(wstring path)
{
	return IsConvertedModelFile(String::ToString(path));
}

bool Path::IsConvertedAnimationFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : ConvertedAnimationFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsConvertedAnimationFile(wstring path)
{
	return IsConvertedAnimationFile(String::ToString(path));
}

bool Path::IsSupportMapFile(string path)
{
	string file_extension = GetExtension(path);
	for (string format : MapFormats)
	{
		if (file_extension == format || file_extension == String::ToUpper(format))
			return true;
	}
	return false;
}

bool Path::IsSupportMapFile(wstring path)
{
	return IsSupportMapFile(String::ToString(path));
}

#pragma endregion

#pragma region 파일존재여부확인영역

void Path::GetDirectoryHierarchy(string path, DirectoryNode * parent)
{
	GetDirectoryHierarchy(String::ToWString(path), parent);
}

void Path::GetDirectoryHierarchy(wstring path, DirectoryNode * parent)
{
	if (!ExistDirectory(path))
		return;
	parent->DirectoryPath = path;
	parent->DirectoryName = GetDirectDirectoryName(path);
	
	wstring file = path + L"*.*";
	WIN32_FIND_DATA findData;
	HANDLE handle = FindFirstFile(file.c_str(), &findData);

	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				if (findData.cFileName[0] != '.')
				{
					wstring folder = path + wstring(findData.cFileName) + L"/";
					DirectoryNode node;
					GetDirectoryHierarchy(folder, &node);

					parent->Children.emplace_back(node);
				}
			}
		} while (FindNextFile(handle, &findData));

		FindClose(handle);
	}
}

bool Path::IsDirectory(string path)
{
	try
	{
		return is_directory(path);
	}
	catch (filesystem_error& error)
	{
		return false;
	}
}

bool Path::IsDirectory(wstring path)
{
	return IsDirectory(String::ToString(path));
}

bool Path::ExistFile(string path)
{
	return ExistFile(String::ToWString(path));
}

bool Path::ExistFile(wstring path)
{
	DWORD fileValue = GetFileAttributes(path.c_str());

	return fileValue < 0xFFFFFFFF;
}

bool Path::ExistDirectory(string path)
{
	return ExistDirectory(String::ToWString(path));
}

bool Path::ExistDirectory(wstring path)
{
	DWORD attribute = GetFileAttributes(path.c_str());

	BOOL temp = (attribute != INVALID_FILE_ATTRIBUTES &&
		(attribute & FILE_ATTRIBUTE_DIRECTORY));

	return temp == TRUE;
}

#pragma endregion

string Path::Combine(string path1, string path2)
{
	return path1 + path2;
}

wstring Path::Combine(wstring path1, wstring path2)
{
	return path1 + path2;
}

string Path::Combine(vector<string> paths)
{
	string temp = "";
	for (string path : paths)
		temp += path;

	return temp;
}

wstring Path::Combine(vector<wstring> paths)
{
	wstring temp = L"";
	for (wstring path : paths)
		temp += path;

	return temp;
}

#pragma region 파일경로관련추출

string Path::GetDirectoryName(string path)
{
	String::Replace(&path, "\\", "/");
	size_t index = path.find_last_of('/');

	return path.substr(0, index + 1);
}

wstring Path::GetDirectoryName(wstring path)
{
	String::Replace(&path, L"\\", L"/");
	size_t index = path.find_last_of('/');

	return path.substr(0, index + 1);
}

string Path::GetDirectDirectoryName(string path)
{
	String::Replace(&path, "\\", "/");
	size_t eindex = path.find_last_of('/');
	int sindex = path.substr(0, eindex).find_last_of('/');
	string result;
	//offset, count임, 시작위치 끝위치 아님
	if (sindex < 0)
		result = path.substr(0, eindex);
	else
		result = path.substr(sindex + 1, eindex - sindex - 1);
	return result;
}

wstring Path::GetDirectDirectoryName(wstring path)
{
	String::Replace(&path, L"\\", L"/");
	size_t eindex = path.find_last_of('/');
	int sindex = path.substr(0, eindex).find_last_of('/');
	wstring result;

	if(sindex<0)
		result= path.substr(0, eindex);
	else
		result= path.substr(sindex+1, eindex- sindex-1);
	return result;
}

string Path::GetExtension(string path)
{
	String::Replace(&path, "\\", "/");
	size_t index = path.find_last_of('.');

	return path.substr(index + 1, path.length());;
}

wstring Path::GetExtension(wstring path)
{
	String::Replace(&path, L"\\", L"/");
	size_t index = path.find_last_of('.');

	return path.substr(index + 1, path.length());;
}

string Path::GetFileName(string path)
{
	String::Replace(&path, "\\", "/");
	size_t index = path.find_last_of('/');

	return path.substr(index + 1, path.length());
}

wstring Path::GetFileName(wstring path)
{
	String::Replace(&path, L"\\", L"/");
	size_t index = path.find_last_of('/');

	return path.substr(index + 1, path.length());
}

string Path::GetFileNameWithoutExtension(string path)
{
	string fileName = GetFileName(path);

	size_t index = fileName.find_last_of('.');
	return fileName.substr(0, index);
}

wstring Path::GetFileNameWithoutExtension(wstring path)
{
	wstring fileName = GetFileName(path);

	size_t index = fileName.find_last_of('.');
	return fileName.substr(0, index);
}

string Path::GetFilePathWithoutExtension(string path)
{
	String::Replace(&path, "\\", "/");
	size_t index = path.find_last_of('.');

	return path.substr(0, index);
}

wstring Path::GetFilePathWithoutExtension(wstring path)
{
	String::Replace(&path, L"\\", L"/");
	size_t index = path.find_last_of('.');

	return path.substr(0, index);
}

#pragma endregion
/////////////////////////////////////////////////////////////////////////////////

const WCHAR* Path::ImageFilter = L"Image\0*.png;*.bmp;*.jpg";
const WCHAR* Path::BinModelFilter = L"Binary Model\0*.model";
const WCHAR* Path::FbxModelFilter = L"Fbx Model\0*.fbx;*.obj\0";
const WCHAR* Path::ConvModelDataFilter = L"Conv Model\0*.material;*.mesh";
const WCHAR* Path::ShaderFilter = L"HLSL file\0*.hlsl";
const WCHAR* Path::XmlFilter = L"Xml file\0*.xml";
const WCHAR* Path::TextFilter = L"Text file\0*.txt";

void Path::OpenFileDialog(wstring file, const WCHAR* filter, wstring folder, function<void(wstring)> func, HWND hwnd)
{
	WCHAR name[255];
	wcscpy_s(name, file.c_str());

	wstring tempFolder = folder;
	String::Replace(&tempFolder, L"/", L"\\");

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = name;
	ofn.lpstrFileTitle = L"불러오기";
	ofn.nMaxFile = 255;
	ofn.lpstrInitialDir = tempFolder.c_str();
	ofn.Flags = OFN_NOCHANGEDIR;
	
	if (GetOpenFileName(&ofn) == TRUE)
	{
		if (func != NULL)
		{
			wstring loadName = name;
			String::Replace(&loadName, L"\\", L"/");
			
			func(loadName);
		}
	}
}

void Path::SaveFileDialog(wstring file, const WCHAR* filter, wstring folder, function<void(wstring)> func, HWND hwnd)
{
	WCHAR name[255];
	wcscpy_s(name, file.c_str());

	wstring tempFolder = folder;
	String::Replace(&tempFolder, L"/", L"\\");

	OPENFILENAME ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = name;
	ofn.lpstrFileTitle = L"저장하기";
	ofn.nMaxFile = 255;
	ofn.lpstrInitialDir = tempFolder.c_str();
	ofn.Flags = OFN_NOCHANGEDIR;

	if (GetSaveFileName(&ofn) == TRUE)
	{
		if (func != NULL)
		{
			wstring loadName = name;
			String::Replace(&loadName, L"\\", L"/");

			func(loadName);
		}
	}
}

void Path::GetFiles(vector<string>* files, string path, string filter, bool bFindSubFolder, bool bFindDirectSubFolder)
{
	vector<wstring> wFiles;
	wstring wPath = String::ToWString(path);
	wstring wFilter = String::ToWString(filter);

	GetFiles(&wFiles, wPath, wFilter, bFindSubFolder, bFindDirectSubFolder);

	for (wstring str : wFiles)
		files->push_back(String::ToString(str));
}

//path : ../Temp/
//filter : *.txt
void Path::GetFiles(vector<wstring>* files, wstring path, wstring filter, bool bFindSubFolder, bool bFindDirectSubFolder)
{
	wstring file = path + filter;

	WIN32_FIND_DATA findData;
	HANDLE handle = FindFirstFile(file.c_str(), &findData);

	if (handle != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (findData.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			{
				if (findData.cFileName[0] != '.')
				{
					wstring folder = path + wstring(findData.cFileName) + L"/";
					if(bFindDirectSubFolder == true)
						files->push_back(folder);
					if(bFindSubFolder == true)
						GetFiles(files, folder, filter, bFindSubFolder, bFindDirectSubFolder);
				}
			}
			else
			{
				wstring fileName = path + wstring(findData.cFileName);
				files->push_back(fileName);
			}
		} while (FindNextFile(handle, &findData));

		FindClose(handle);
	}
}

bool Path::IsRelativePath(string path)
{
	return IsRelativePath(String::ToWString(path));
}

bool Path::IsRelativePath(wstring path)
{
	BOOL b = PathIsRelative(path.c_str());

	return b >= TRUE;
}

#pragma region 생성삭제카피

void Path::CreateFolder(string path)
{
	CreateFolder(String::ToWString(path));
}

void Path::CreateFolder(wstring path)
{
	if (ExistDirectory(path) == false)
		CreateDirectory(path.c_str(), NULL);
}

void Path::CreateFolders(string path)
{
	CreateFolders(String::ToWString(path));
}

void Path::CreateFolders(wstring path)
{
	String::Replace(&path, L"\\", L"/");

	vector<wstring> folders;
	String::SplitString(&folders, path, L"/");

	wstring temp = L"";
	for (wstring folder : folders)
	{
		temp += folder + L"/";

		CreateFolder(temp);
	}
}

bool Path::DeleteFolder(const string & path)
{
	return DeleteFolder(String::ToWString(path));
}

bool Path::DeleteFolder(const wstring & path)
{
	bool result = false;

	try
	{
		result = remove_all(path) > 0;
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return result;
}

bool Path::Delete_File(const string & path)
{
	return Delete_File(String::ToWString(path));
}

bool Path::Delete_File(const wstring & path)
{
	bool result = false;

	try
	{
		result = remove(path);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return result;
}

bool Path::Copy_File(const string & src, const string & dst)
{
	return Copy_File(String::ToWString(src), String::ToWString(dst));
}

bool Path::Copy_File(const wstring & src, const wstring & dst)
{
	if (src == dst)
		return false;

	if (!ExistDirectory(GetDirectoryName(dst)))
		CreateFolder(GetDirectoryName(dst));

	bool result = false;

	try
	{
		result = copy_file(src, dst, copy_options::overwrite_existing);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return result;
}

#pragma endregion
