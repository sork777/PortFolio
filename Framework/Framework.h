#pragma once

#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#endif

#include <Windows.h>
#include <assert.h>

//STL
#include <string>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <functional>
#include <iterator>
#include <thread>
#include <mutex>
#include <type_traits>
#include <iostream>

using namespace std;

//Direct3D
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>		//hlsl 컴파일러?
#include <d3d11shader.h>
#include <d3d11.h>
#include <d3dx10math.h>
#include <d3dx11async.h>
#include <d3dx11effect.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "Effects11d.lib")

//ImGui
#include <ImGui_New/imgui.h>
#include <ImGui_New/imgui_impl_dx11.h>
#include <ImGui_New/imgui_impl_win32.h>
#pragma comment(lib, "ImGui_New/imgui.lib")

//DirectXTex
#include <DirectXTex.h>
#pragma comment(lib, "directxtex.lib")

//Assimp
#include "Assimp/Importer.hpp"	//h랑 cpp 합친것
#include "Assimp/scene.h"	
#include "Assimp/postprocess.h"

#pragma comment(lib, "Assimp/assimp-vc140-mt.lib")

#define Check(hr) { assert(SUCCEEDED(hr)); }
#define Super __super

#define SafeRelease(p){ if(p){ (p)->Release(); (p) = NULL; } }
#define SafeDelete(p){ if(p){ delete (p); (p) = NULL; } }
#define SafeDeleteArray(p){ if(p){ delete [] (p); (p) = NULL; } }

typedef D3DXVECTOR2 Vector2;
typedef D3DXVECTOR3 Vector3;
typedef D3DXVECTOR4 Vector4;
typedef D3DXCOLOR Color;
typedef D3DXMATRIX Matrix;
typedef D3DXQUATERNION Quaternion;
typedef D3DXPLANE Plane;

//순서 중요
#include "Systems/D3D.h"
#include "Systems/Keyboard.h"
#include "Systems/Mouse.h"
#include "Systems/Time.h"
#include "Systems/Gui.h"

#include "Viewer/Viewport.h"
#include "Viewer/Projection.h"
#include "Viewer/Perspective.h"
#include "Viewer/Orthographic.h"
#include "Viewer/Camera.h"
#include "Viewer/RenderTarget.h"
#include "Viewer/DepthStencil.h"
#include "Viewer/Fixity.h"
#include "Viewer/Frustum.h"

#include "Renders/Buffers.h"
#include "Renders/Shader.h"
#include "Renders/Texture.h"
#include "Renders/VertexLayouts.h"
#include "Renders/Context.h"
#include "Renders/Renderer.h"

#include "Components/ObjectBase/ObjectBaseComponent.h"
//#include "Components/ObjectBase/AnimationComponent.h"	
#include "Components/ObjectBase/ModelMeshComponent.h"	
#include "Components/ObjectBase/FigureMeshComponent.h"	
#include "Components/ObjectBase/CameraComponent.h"	
#include "Components/ObjectBase/OBB_CollisionComponent.h"	

#include "Components/CommonComponent.h"	
#include "Components/Common/Material.h"	
#include "Components/Common/MaterialPBR.h"	
#include "Components/Common/PerFrame.h"
#include "Components/Common/Transform.h"

#include "Utilities/DebugLine.h"
#include "Utilities/Math.h"
#include "Utilities/String.h"
#include "Utilities/Path.h"
#include "Utilities/BinaryFile.h"

#include "Collision/Collider.h"
#include "Collision/OBBCollider.h"
//#include "Collision/SphereCollider.h"
//#include "Collision/CapsuleCollider.h"

#include "Meshes/Mesh.h"
#include "Meshes/MeshRender.h"
#include "Meshes/MeshQuad.h"
#include "Meshes/MeshGrid.h"
#include "Meshes/MeshCube.h"
#include "Meshes/MeshCylinder.h"
#include "Meshes/MeshSphere.h"

#include "Model/Model.h"
#include "Model/ModelMesh.h"
#include "Model/ModelClip.h"
#include "Model/ModelRender.h"
#include "Model/ModelAnimator.h"

#include "Model/Loader/Types.h"
#include "Model/Loader/Converter.h"

#include "Objects/BaseObject.h"
#include "Objects/Actor/Actor.h"
#include "Objects/Etc/BillBoardObj.h"

#include "LightEffects/Shadow.h"
#include "LightEffects/Light.h"
#include "LightEffects/CSM.h"

#include "ScreenSpace/Render2D.h"
#include "ScreenSpace/GBuffer.h"
#include "ScreenSpace/SSAO.h"
#include "ScreenSpace/SSLR.h"

#include "ImguiWidget/IWidget.h"
#include "ImguiWidget/Widget_Contents.h"

#include "Effect/TrailRenderer.h"
#include "Effect/ModelIllusion.h"

#include "Assets/ContentsAsset.h"
#include "Assets/ContentsAsset/ModelAsset.h"
#include "Assets/ContentsAsset/TextureAsset.h"

#include "Assets/AssetManager.h"

#include "Editor/AnimationEditor.h"
#include "Editor/ParticleEditor.h"
#include "Editor/ActorEditor.h"
