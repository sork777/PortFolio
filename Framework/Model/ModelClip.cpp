#include "Framework.h"
#include "ModelClip.h"

ModelClip::ModelClip()
{

}

ModelClip::ModelClip(const ModelClip & clip)
{
	name = clip.name;
	fileName = clip.fileName;

	duration = clip.duration;
	frameRate = clip.frameRate;
	frameCount = clip.frameCount;

	unordered_map<wstring, ModelKeyframe *> clipMap = clip.keyframeMap;
	unordered_map<wstring, ModelKeyframe *>::iterator iter;
	for (iter = clipMap.begin(); iter != clipMap.end(); iter++)
	{
		ModelKeyframe* curframe = iter->second;
		ModelKeyframe* newframe = new ModelKeyframe();
		newframe->BoneName = curframe->BoneName;
		newframe->Transforms = curframe->Transforms;
		keyframeMap[iter->first] = newframe;
	}
}

ModelClip::~ModelClip()
{

}

ModelKeyframe * ModelClip::Keyframe(wstring name)
{
	if (keyframeMap.count(name) < 1)
		return NULL;

	return keyframeMap[name];
}
