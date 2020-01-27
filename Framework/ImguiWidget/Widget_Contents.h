#pragma once
#include "IWidget.h"
#include "WidgetUtility/DragDrop.h"
#include "WidgetUtility/IconProvider.h"

struct ContentNode
{
	Item* item;
	vector<ContentNode*> childs;
};

class Widget_Contents : public IWidget
{
public:
	Widget_Contents();
	virtual ~Widget_Contents();

	void Initialize();
	void Render() override;

	void SetImport(function<void(wstring)> func) { 
		ImportFunc = func; 
	}
private:
	void ShowItems();
	void DirectoryViewer();
	void ChildDirViewer(DirectoryNode * node,int& index);

private:
	void InitailizeNode(ContentNode* root);
	void SelectIconFromFile(const string& path);

	void UpdateItems(const string& path);
	DragDropPayloadType GetPayloadType(IconType& type);

private:
	function<void(wstring)> ImportFunc = NULL;
private:
	string currentPath;
	string rootPath;
	float itemSize;
	float itemSizeMin;
	float itemSizeMax;
	
	int selectedDirectory=0;

	DirectoryNode dirHierarchyRoot;
	ContentNode* contentsRoot;
	vector<struct Item> items;
	vector<struct Item> icons;
	vector< DragDropPayloadType> MovableTypes;
};