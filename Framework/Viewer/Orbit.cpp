#include "Framework.h"
#include "Orbit.h"


Orbit::Orbit(const float& rad, const float& minRad, const float& maxRad)
	: rad(rad), minRad(minRad), maxRad(maxRad),
	rSpeed(2),phi(0),theta(0),
	objPos(0,0,0)
{
}


Orbit::~Orbit()
{
}

void Orbit::Update()
{
	Vector3 position;

	Position(&position);

	//각도 회전에 따른 위치 이동
	position = objPos + D3DXVECTOR3
	(
		(rad * sinf(phi) * cosf(theta)),
		(rad * cosf(phi)),
		(rad * sinf(phi) * sinf(theta))
	);
	OrbitUp();

	Position(position);
}

void Orbit::Speed(float rSpeed)
{
	this->rSpeed = rSpeed;
}

void Orbit::SetRad(const float & rad, const float & minRad, const float & maxRad)
{
	this->rad = rad;
	this->minRad = minRad;
	this->maxRad = maxRad;
}


//밖에서 받은 마우스좌표를 통해 각도/반지름 수정
void Orbit::CameraMove(Vector3& val)
{
	//카메라와 오브젝트간의 거리(반지름)
	//반지름이 최소 반지름에서 최대 반지름 사이면 휠로 이동
	//반지름 제한
	if (val.z < minRad)
		val.z = minRad;
	else if (val.z > maxRad)
		val.z = maxRad;
	
	//범위 제한(윗점과 아랫점 기준으로 제한
	val.y = Math::Clamp(val.y, 0.05f, 0.95f);
	////각도 보정
	const float factor = 2.0f;
	if (val.x >= 2.0f*factor)
		val.x -= 2.0f*factor;
	else if (val.x < 0.0f)
		val.x += 2.0f*factor;


	rad = val.z;
	phi = Math::PI * 0.5f*val.y;
	theta = Math::PI/ factor *val.x;

}

void Orbit::OrbitUp()
{
	//phi,theta인 점에서의 기울기이자 up벡터를 구하기 위함 항상 위로 갈것
	//현재 포지션과 구에서의 같은 수직 궤도 상에서 아래쪽
	Vector3 p0((sinf(phi + 0.1f) * cosf(theta)),
		(cosf(phi + 0.1f)),
		(sinf(phi + 0.1f) * sinf(theta)));
	//현재 포지션과 구에서의 같은 수직 궤도 상에서 위쪽
	Vector3 p1((sinf(phi - 0.1f) * cosf(theta)),
		(cosf(phi - 0.1f)),
		(sinf(phi - 0.1f) * sinf(theta)));
	
	D3DXVec3Normalize(&oUp, &(p1 - p0));
}

void Orbit::View()
{
	Vector3 position;
	Position(&position);
	Matrix matView;
	   
	//up 값은 회전을 통한 카메라의 up값
	//바라볼 객체를 통해 위치 조정
	D3DXMatrixLookAtLH(&matView, &position, &objPos, &oUp);
	//조정한 행렬 세팅
	SetMatrix(matView);
}

void Orbit::Property()
{

	Vector3 position;
	Position(&position);
	
	if (ImGui::CollapsingHeader("Camera Data", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize,1.0f);
		ImGui::BeginChild("##Transform", ImVec2(ImGui::GetWindowContentRegionWidth(), 180),true);
		{
			ImGui::Text("ObjectPosition");
			ImGui::Text("%.2f, %.2f, %.2f"				, objPos.x, objPos.y, objPos.z);
			ImGui::Separator();
			ImGui::Text("CameraPosition");
			ImGui::Text("%.2f, %.2f, %.2f"				, position.x, position.y, position.z);
			ImGui::Text("CameraAngle");
			ImGui::Text("Phi : %.2f, Theta : %.2f"		, Math::ToDegree(phi), Math::ToDegree(theta));
			ImGui::Separator();
			ImGui::Text("Distance Camera to Object");
			ImGui::LabelText("##Distance", "%.2f"		, D3DXVec3Length(&(position - objPos)));
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();
	}
}

void Orbit::ViewCameraArea()
{
	UINT stackCount = 20;
	float thetaStep = 2.0f * Math::PI / stackCount;

	vector<Vector3> v, v2, v3;
	for (UINT i = 0; i <= stackCount; i++)
	{
		float theta = i * thetaStep;

		Vector3 p = Vector3
		(
			(rad * cosf(theta)),
			0,
			(rad * sinf(theta))
		);
		Vector3 p2 = Vector3
		(
			(rad * cosf(theta)),
			(rad * sinf(theta)),
			0
		);
		Vector3 p3 = Vector3
		(
			0,
			(rad * cosf(theta)),
			(rad * sinf(theta))
		);
		p += objPos;
		p2 += objPos;
		p3 += objPos;
		v.emplace_back(p);
		v2.emplace_back(p2);
		v3.emplace_back(p3);
	}
	for (UINT i = 0; i < stackCount; i++)
	{
		DebugLine::Get()->RenderLine(v[i], v[i + 1]);
		DebugLine::Get()->RenderLine(v2[i], v2[i + 1]);
		DebugLine::Get()->RenderLine(v3[i], v3[i + 1]);
	}
}