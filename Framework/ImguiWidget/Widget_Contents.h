#pragma once
#include "IWidget.h"
#include "WidgetUtility/DragDrop.h"
#include "WidgetUtility/IconProvider.h"

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
	void ChildDirViewer(DirectoryHierarchy * node,int& index);

private:
	void UpdateItems(const string& path);
	void SelectIconFromFile(const string& path);
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

	DirectoryHierarchy dirHierarchyRoot;
	vector<struct Item> items;
	vector<struct Item> icons;
	vector< DragDropPayloadType> MovableTypes;
};