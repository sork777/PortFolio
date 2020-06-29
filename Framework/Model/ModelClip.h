#pragma once

struct ModelKeyframeData
{
	float Time;

	Vector3 Scale;
	Quaternion Rotation;
	Vector3 Translation;
};

struct ModelKeyframe
{
	wstring BoneName;
	vector<ModelKeyframeData> Transforms;
};

class ModelClip
{
public:
	friend class ModelAnimator;

private:
	ModelClip();
	ModelClip(const ModelClip& clip);
	~ModelClip();

public:
	float Duration() { return duration; }
	float FrameRate() { return frameRate; }
	UINT FrameCount() { return frameCount; }
	const wstring& Name() { return name; }
	const wstring& FileName() { return fileName; }
	//const wstring& FilePath() { return filePath; }
	//const wstring& DirPath() { return dirPath; }
	
	ModelKeyframe* Keyframe(wstring name);

private:
	wstring name;
	wstring fileName;
	//wstring filePath;
	//wstring dirPath;

	float duration;
	float frameRate;
	UINT frameCount;

	unordered_map<wstring, ModelKeyframe *> keyframeMap;
};