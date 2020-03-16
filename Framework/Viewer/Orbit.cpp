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
	CameraMove();
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


void Orbit::CameraMove()
{

	Vector3 position;

	Position(&position);

	//���콺 ���� ,z���� �� �̵�
	Vector3 val = Mouse::Get()->GetMoveValue();

	//ī�޶�� ������Ʈ���� �Ÿ�(������)


	//�������� �ּ� ���������� �ִ� ������ ���̸� �ٷ� �̵�
	rad += val.z *Time::Delta();
	//������ ����
	if (rad < minRad)
		rad = minRad;
	else if (rad > maxRad)
		rad = maxRad;

	Vector3 R;
	Rotation(&R);

	//����Ʈ ������ ��ȯ ��.
	if (Mouse::Get()->Press(1) == true)
	{
		phi -= Math::PI * 0.5f*val.y*Time::Delta();
		theta += Math::PI * 0.5f*val.x*Time::Delta();
	}
	//���� ����(������ �Ʒ��� �������� ����

	phi = Math::Clamp(phi, 0.1f, Math::PI - 0.1f);

	////���� ����

	if (theta >= 2.0f*Math::PI)
		theta -= 2.0f*Math::PI;
	else if (theta < 0.0f)
		theta += 2.0f*Math::PI;

	//���� ȸ���� ���� ��ġ �̵�
	position = objPos + D3DXVECTOR3
	(
		(rad * sinf(phi) * cosf(theta)),
		(rad * cosf(phi)),
		(rad * sinf(phi) * sinf(theta))
	);
	OrbitUp();

	Position(position);
}

void Orbit::OrbitUp()
{
	//phi,theta�� �������� �������� up���͸� ���ϱ� ���� �׻� ���� ����
	//���� �����ǰ� �������� ���� ���� �˵� �󿡼� �Ʒ���
	Vector3 p0((sinf(phi + 0.1f) * cosf(theta)),
		(cosf(phi + 0.1f)),
		(sinf(phi + 0.1f) * sinf(theta)));
	//���� �����ǰ� �������� ���� ���� �˵� �󿡼� ����
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
	   
	//up ���� ȸ���� ���� ī�޶��� up��
	//�ٶ� ��ü�� ���� ��ġ ����
	D3DXMatrixLookAtLH(&matView, &position, &objPos, &oUp);
	//������ ��� ����
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