#include "Framework.h"
#include "Viewport.h"

Viewport::Viewport(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	Set(width, height, x, y, minDepth, maxDepth);
}

Viewport::~Viewport()
{
	
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	viewport.TopLeftX = this->x = x;
	viewport.TopLeftY = this->y = y;
	viewport.Width = this->width = width;
	viewport.Height = this->height = height;
	viewport.MinDepth = this->minDepth = minDepth;
	viewport.MaxDepth = this->maxDepth = maxDepth;

	RSSetViewport();
}

void Viewport::GetRay(OUT Vector3 * position, OUT Vector3 * direction, Matrix & W, Matrix & V, Matrix & P)
{
	Vector3 mouse = Mouse::Get()->GetPosition();

	Vector2 point;
	//inv Viewport
	{
		//			���⼭ 0~2�� ����			���� -1~1�̵�
		point.x = ((2.0f*mouse.x) / width) - 1.0f;
		//���콺�� y��ǥ���� ��¤�� ����
		point.y = (((2.0f*mouse.y) / height) - 1.0f) * -1.0f;
	}

	//inv Projection
	{
		point.x = point.x / P._11;
		point.y = point.y / P._22;
	}

	
	Vector3 cameraPosition;
	//inv View
	{
		//������ View
		Matrix invView;
		D3DXMatrixInverse(&invView, NULL, &V);

		//V(View)�� ��ȯ �Ǳ����� ī�޶��� ��ġ
		//ī�޶��� ��ġ������ View����� ����Ƿ� invView�� 4���� ī�޶� ���� ��ġ�� �ȴ�.
		cameraPosition = Vector3(invView._41, invView._42, invView._43);
		//��� ī�޶� ��ġ �׳� �����͵� �ȴ�. �׳� ���� �˶�� �Ѱ�.

		//TODO: ���� �ٽ� ����
		//ȭ���� �þ߰���-view-�̶�� �ϸ� �߾�(�þ�-view- �߽�)���� ��ǥ������ ������ ����
		D3DXVec3TransformNormal(direction, &Vector3(point.x, point.y, 1), &invView);
		D3DXVec3Normalize(direction, direction);
	}

	//inv World
	{
		//������ world
		Matrix invWorld;
		D3DXMatrixInverse(&invWorld, NULL, &W);
		
		//ī�޶� ��ġ�� ���� ��������
		//3D������ ī�޶� ��ġ�� ����?
		//view������ world�������� ī�޶� �����϶� ���� ���Ҷ� ���� ���Ѵٴ� ���� ��������.
		D3DXVec3TransformCoord(position, &cameraPosition, &invWorld);
		D3DXVec3TransformNormal(direction, direction, &invWorld);
		D3DXVec3Normalize(direction, direction);
	}
	//���� ���� �϶�� ������. ������ WVP ���ϰ� ����� ���ϸ� ��


}

void Viewport::Project(OUT Vector3 * position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{
	Matrix wvp = W * V*P;
	Vector3 temp = source;
	D3DXVec3TransformCoord(position, &temp, &wvp);
	//����Ʈ�� x��
	position->x = ((position->x + 1.0f) *0.5f)*width + x;
	position->y = ((-position->y + 1.0f) *0.5f)*height + y;
	//��� �Ǳ���? ����ȭ �Ǽ� 0~1��
	position->z = (position->z * (maxDepth - minDepth)) + minDepth;
}

void Viewport::Unproject(OUT Vector3 * position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{	
	Vector3 temp = source;
	//����Ʈ�� x��
	position->x = ((temp.x - x) / width)*2.0f - 1.0f;
	position->y = (((temp.y - y) / height)*2.0f - 1.0f) * -1.0f;
	//��� �Ǳ���? ����ȭ �Ǽ� 0~1��
	position->z = (temp.z - minDepth) / (maxDepth - minDepth);

	Matrix wvp = W * V*P;
	D3DXMatrixInverse(&wvp, NULL, &wvp);
		
	//������ ī�޶� �������� ���ֱ� ������ ȭ�鿡�� �Ⱥ���?
	D3DXVec3TransformCoord(position, position, &wvp);


}


void Viewport::RSSetViewport()
{
	D3D::GetDC()->RSSetViewports(1, &viewport);
}