#pragma once
#include "./ImGui_New/imgui.h"
#include "Utilities/ImGuizmo.h"

struct GuiText
{
	D3DXVECTOR2 Position;
	D3DXCOLOR Color;
	string Content;

	GuiText()
	{

	}
};

struct GuiTexture
{
	Vector2 Position;
	Vector2 Size;
	D3DXCOLOR Color;
	class Texture* Content;

	GuiTexture() {	}
	
};
class Gui
{
public:
	static void Create();
	static void Delete();

	static Gui* Get();

	LRESULT MsgProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam);
	void Resize();

	void Update();
	void Render();

	void AddWidget(class Widget* widget);

	void RenderText(GuiText& text);
	void RenderText(float x, float y, string content);
	void RenderText(float x, float y, float r, float g, float b, string content);
	void RenderText(D3DXVECTOR2 position, D3DXCOLOR color, string content);

	void RenderGUITexture(GuiTexture& text);
	void RenderGUITexture(float x, float y, Texture* content);
	void RenderGUITexture(float x, float y, float u, float v, D3DXCOLOR color, Texture* content);
	void RenderGUITexture(Vector2 position, Vector2 size, D3DXCOLOR color, Texture* content);
	
	void SetGizmo(class Transform* selectedTransform, class Transform* vTransform = NULL, bool bReverse=false);
	void RenderGizmo();
private:
	Gui();
	~Gui();

private:
	void ApplyStyle();
	void DockingPannel();

private:
	static Gui* instance;
	vector<class Widget *> widgets;

	vector<GuiText> texts;
	vector<GuiTexture> textures;

	ImGuizmo::OPERATION operation=ImGuizmo::TRANSLATE;
	ImGuizmo::MODE mode=ImGuizmo::WORLD;
	class Transform* selectedTransform=NULL;
	class Transform* vTransform=NULL;
	bool bReverse;
};