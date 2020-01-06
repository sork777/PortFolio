#include "Framework.h"
#include "Freedom.h"

Freedom::Freedom()
	:move(20),rot(2), bMove(true)
{
	
}


Freedom::~Freedom()
{
}

void Freedom::Update()
{
	if (Keyboard::Get()->Down('P'))
		bMove = !bMove;
	if (bMove == false)
		return;

	if (Mouse::Get()->Press(1) == false)
		return;

	const Vector3& f = Forward();
	const Vector3& r = Right();
	const Vector3& u = Up();
	
	Vector3 position;
	
	//원래 포지션 받아오기
	Position(&position);

	if (Keyboard::Get()->Press('W'))
		position += f * move*Time::Delta();
	else if (Keyboard::Get()->Press('S'))
		position -= f * move*Time::Delta();

	if (Keyboard::Get()->Press('D'))
		position += r * move*Time::Delta();
	else if (Keyboard::Get()->Press('A'))
		position -= r * move*Time::Delta();

	if (Keyboard::Get()->Press('E'))
		position += u * move*Time::Delta();
	else if (Keyboard::Get()->Press('Q'))
		position -= u * move*Time::Delta();

	//부모에서 포지션 갱신
	Position(position);

	Vector3 R;
	Rotation(&R);

	Vector3 val = Mouse::Get()->GetMoveValue();//마우스 이동속도
	R.x += val.y*rot*Time::Delta();
	R.y += val.x*rot*Time::Delta();
	R.z=0.0f;
	//ImGui::SliderFloat3("Rotation", (float*)&R,-100,100);
	Rotation(R);
}

void Freedom::Speed(float move, float rot)
{
	this->move = move;
	this->rot = rot;
}
