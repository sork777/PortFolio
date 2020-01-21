#include "Framework.h"
#include "Gui.h"
//#include "Widget.h"

Gui* Gui::instance = NULL;

void Gui::Create()
{
	assert(instance == NULL);

	instance = new Gui();
}

void Gui::Delete()
{
	SafeDelete(instance);
}

Gui * Gui::Get()
{
	return instance;
}

LRESULT Gui::MsgProc(HWND handle, UINT message, WPARAM wParam, LPARAM lParam)
{
	return ImGui_ImplWin32_Proc(handle, message, wParam, lParam);
}

void Gui::Resize()
{
	ImGui_ImplDX11_InvalidateDeviceObjects();
	ImGui_ImplDX11_CreateDeviceObjects();
}

void Gui::Update()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void Gui::Render()
{
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowBgAlpha(0.0f);

	ImGui::Begin
	(
		"TextWindow", NULL,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoScrollWithMouse |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoInputs |
		ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoNavFocus
	);

	for (GuiText text : texts)
	{
		ImVec2 position = ImVec2(text.Position.x, text.Position.y);
		ImColor color = ImColor(text.Color.r, text.Color.g, text.Color.b, text.Color.a);

		ImGui::GetWindowDrawList()->AddText(position, color, text.Content.c_str());
	}
	texts.clear();
	for (GuiTexture texture : textures)
	{
		ImVec2 start = ImVec2((texture.Position - texture.Size*0.5f).x, (texture.Position - texture.Size*0.5f).y);
		ImVec2 end = ImVec2((texture.Position + texture.Size*0.5f).x, (texture.Position + texture.Size*0.5f).y);
		
		ImColor color = ImColor(texture.Color.r, texture.Color.g, texture.Color.b, texture.Color.a);

		ImGui::GetWindowDrawList()->AddImage(texture.Content?texture.Content->SRV():NULL,start,end ,ImVec2(0,0), ImVec2(1, 1), color );
	}
	textures.clear();
	RenderGizmo();

	ImGui::End();


	//DockingPannel();

	/*for (Widget* widget : widgets)
	{
	if (widget->Visible() == true)
	{
	widget->Begin();
	widget->Render();
	widget->End();
	}
	}*/

	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}
}

void Gui::AddWidget(Widget * widget)
{
	widgets.push_back(widget);
}

void Gui::RenderText(GuiText & text)
{
	texts.push_back(text);
}

void Gui::RenderText(float x, float y, string content)
{
	GuiText text;
	text.Position = D3DXVECTOR2(x, y);
	text.Color = D3DXCOLOR(1, 1, 1, 1);
	text.Content = content;

	RenderText(text);
}

void Gui::RenderText(float x, float y, float r, float g, float b, string content)
{
	GuiText text;
	text.Position = D3DXVECTOR2(x, y);
	text.Color = D3DXCOLOR(r, g, b, 1);
	text.Content = content;

	RenderText(text);
}

void Gui::RenderText(D3DXVECTOR2 position, D3DXCOLOR color, string content)
{
	GuiText text;
	text.Position = position;
	text.Color = color;
	text.Content = content;

	RenderText(text);
}

void Gui::RenderGUITexture(GuiTexture & text)
{
	textures.push_back(text);
}

void Gui::RenderGUITexture(float x, float y, Texture * content)
{
	GuiTexture text;
	text.Position = Vector2(x, y);
	text.Color = D3DXCOLOR(1, 1, 1, 1);
	text.Size = Vector2(100, 100);
	text.Content = content;

	RenderGUITexture(text);
}

void Gui::RenderGUITexture(float x, float y, float u, float v, D3DXCOLOR color, Texture * content)
{
	GuiTexture text;
	text.Position = Vector2(x, y);
	text.Color = color;
	text.Size = Vector2(u,v);
	text.Content = content;

	RenderGUITexture(text);
}

void Gui::RenderGUITexture(Vector2 position, Vector2 size, D3DXCOLOR color, Texture * content)
{
	GuiTexture text;
	text.Position = position;
	text.Color = color;
	text.Size = size;
	text.Content = content;

	RenderGUITexture(text);
}

void Gui::SetGizmo(Transform * selectedTransform, class Transform* vTransform)
{	
	this->selectedTransform = selectedTransform;
	this->vTransform = vTransform;
}

void Gui::RenderGizmo()
{
	if (selectedTransform == NULL)
		return;

	if (ImGui::IsKeyPressed(87)) // w
		operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69)) // e
		operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r
		operation = ImGuizmo::SCALE;

	Matrix vMatrix,InvMat;
	Matrix matrix = selectedTransform->World();
	if (vTransform == NULL)
	{
		vMatrix = matrix;
		D3DXMatrixIdentity(&InvMat);
	}
	else
	{
		vTransform->Parent(matrix);
		vMatrix = vTransform->Local();
		D3DXMatrixInverse(&InvMat,NULL, &vMatrix);
		vMatrix = vTransform->World();
	}

	float width = D3D::Get()->Width();
	float height = D3D::Get()->Height();
	Matrix view = Context::Get()->View();
	Matrix proj = Context::Get()->Projection();


	ImGuizmo::SetOrthographic(true);
	ImGuizmo::SetDrawlist();
	ImGuizmo::SetRect(0, 0, width, height);
	ImGuizmo::Manipulate
	(
		view,
		proj,
		operation,
		mode,
		vMatrix
	);
	matrix = InvMat * vMatrix;
	selectedTransform->World(matrix);	

	int a = 0;
}

Gui::Gui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
	//io.ConfigFlags |= ImGuiConfigFlags_ViewportsNoTaskBarIcon;
	//io.ConfigResizeWindowsFromEdges = true;

	D3DDesc desc = D3D::GetDesc();

	ImGui_ImplWin32_Init(desc.Handle);
	ImGui_ImplDX11_Init(D3D::GetDevice(), D3D::GetDC());

	ApplyStyle();
}

Gui::~Gui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Gui::ApplyStyle()
{
	ImGui::GetIO().ConfigWindowsResizeFromEdges = true;
	ImGui::StyleColorsDark();
	ImGuiStyle& style = ImGui::GetStyle();


	float fontSize = 15.0f;
	float roundness = 5.0f;
	ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	ImVec4 text = ImVec4(0.76f, 0.77f, 0.8f, 1.0f);
	ImVec4 black = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	ImVec4 backgroundVeryDark = ImVec4(0.08f, 0.086f, 0.094f, 1.00f);
	ImVec4 backgroundDark = ImVec4(0.117f, 0.121f, 0.145f, 1.00f);
	ImVec4 backgroundMedium = ImVec4(0.26f, 0.26f, 0.27f, 1.0f);
	ImVec4 backgroundLight = ImVec4(0.37f, 0.38f, 0.39f, 1.0f);
	ImVec4 highlightBlue = ImVec4(0.172f, 0.239f, 0.341f, 1.0f);
	ImVec4 highlightBlueHovered = ImVec4(0.202f, 0.269f, 0.391f, 1.0f);
	ImVec4 highlightBlueActive = ImVec4(0.382f, 0.449f, 0.561f, 1.0f);
	ImVec4 barBackground = ImVec4(0.078f, 0.082f, 0.09f, 1.0f);
	ImVec4 bar = ImVec4(0.164f, 0.180f, 0.231f, 1.0f);
	ImVec4 barHovered = ImVec4(0.411f, 0.411f, 0.411f, 1.0f);
	ImVec4 barActive = ImVec4(0.337f, 0.337f, 0.368f, 1.0f);

	// Spatial
	style.WindowBorderSize = 1.0f;
	style.FrameBorderSize = 1.0f;
	style.FramePadding = ImVec2(5, 5);
	style.ItemSpacing = ImVec2(6, 5);
	style.Alpha = 1.0f;
	style.WindowRounding = roundness;
	style.FrameRounding = roundness;
	style.PopupRounding = roundness;
	style.GrabRounding = roundness;
	style.ScrollbarSize = 20.0f;
	style.ScrollbarRounding = roundness;

	// Colors
	style.Colors[ImGuiCol_Text] = text;
	style.Colors[ImGuiCol_WindowBg] = backgroundDark;
	style.Colors[ImGuiCol_Border] = black;
	style.Colors[ImGuiCol_FrameBg] = bar;
	style.Colors[ImGuiCol_FrameBgHovered] = highlightBlue;
	style.Colors[ImGuiCol_FrameBgActive] = highlightBlueHovered;
	style.Colors[ImGuiCol_TitleBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_TitleBgActive] = bar;
	style.Colors[ImGuiCol_MenuBarBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_ScrollbarBg] = barBackground;
	style.Colors[ImGuiCol_ScrollbarGrab] = bar;
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = barHovered;
	style.Colors[ImGuiCol_ScrollbarGrabActive] = barActive;
	style.Colors[ImGuiCol_CheckMark] = white;
	style.Colors[ImGuiCol_SliderGrab] = bar;
	style.Colors[ImGuiCol_SliderGrabActive] = barActive;
	style.Colors[ImGuiCol_Button] = barActive;
	style.Colors[ImGuiCol_ButtonHovered] = highlightBlue;
	style.Colors[ImGuiCol_ButtonActive] = highlightBlueActive;
	style.Colors[ImGuiCol_Header] = highlightBlue; // selected items (tree, menu bar etc.)
	style.Colors[ImGuiCol_HeaderHovered] = highlightBlueHovered; // hovered items (tree, menu bar etc.)
	style.Colors[ImGuiCol_HeaderActive] = highlightBlueActive;
	style.Colors[ImGuiCol_Separator] = backgroundLight;
	style.Colors[ImGuiCol_ResizeGrip] = backgroundMedium;
	style.Colors[ImGuiCol_ResizeGripHovered] = highlightBlue;
	style.Colors[ImGuiCol_ResizeGripActive] = highlightBlueHovered;
	style.Colors[ImGuiCol_PlotLines] = ImVec4(0.0f, 0.7f, 0.77f, 1.0f);
	style.Colors[ImGuiCol_PlotHistogram] = highlightBlue; // Also used for progress bar
	style.Colors[ImGuiCol_PlotHistogramHovered] = highlightBlueHovered;
	style.Colors[ImGuiCol_TextSelectedBg] = highlightBlue;
	style.Colors[ImGuiCol_PopupBg] = backgroundVeryDark;
	style.Colors[ImGuiCol_DragDropTarget] = backgroundLight;
}

void Gui::DockingPannel()
{
	ImGuiWindowFlags windowFlags =
		ImGuiWindowFlags_MenuBar |
		ImGuiWindowFlags_NoDocking |
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoCollapse |
		//ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNavFocus;

	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->Pos);
	ImGui::SetNextWindowSize(viewport->Size);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::SetNextWindowBgAlpha(0.0f);


	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	bool bDocking = true;
	ImGui::Begin("DockingPannel", &bDocking, windowFlags);
	{
		ImGui::PopStyleVar(3);

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID pannelID = ImGui::GetID("Pannel");
			ImGui::DockSpace(pannelID, ImVec2(0.0f, 0.0f), ImGuiWindowFlags_AlwaysAutoResize);
		}
	}
	ImGui::End();
}
