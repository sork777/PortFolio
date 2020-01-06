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
		//			여기서 0~2가 나옴			빼면 -1~1이됨
		point.x = ((2.0f*mouse.x) / width) - 1.0f;
		//마우스의 y좌표값은 뒤짚혀 있음
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
		//원래의 View
		Matrix invView;
		D3DXMatrixInverse(&invView, NULL, &V);

		//V(View)로 변환 되기전에 카메라의 위치
		//카메라의 위치가지고 View행렬을 만드므로 invView의 4행은 카메라 원래 위치가 된다.
		cameraPosition = Vector3(invView._41, invView._42, invView._43);
		//사실 카메라 위치 그냥 가져와도 된다. 그냥 원리 알라고 한것.

		//TODO: 설명 다시 보기
		//화면을 시야공간-view-이라고 하면 중앙(시야-view- 중심)에서 목표점으로 나가는 방향
		D3DXVec3TransformNormal(direction, &Vector3(point.x, point.y, 1), &invView);
		D3DXVec3Normalize(direction, direction);
	}

	//inv World
	{
		//원래의 world
		Matrix invWorld;
		D3DXMatrixInverse(&invWorld, NULL, &W);
		
		//카메라 위치를 월드 공간으로
		//3D공간의 카메라 위치와 방향?
		//view공간이 world공간에서 카메라 움직일때 마다 변할때 마다 변한다는 점을 생각하자.
		D3DXVec3TransformCoord(position, &cameraPosition, &invWorld);
		D3DXVec3TransformNormal(direction, direction, &invWorld);
		D3DXVec3Normalize(direction, direction);
	}
	//원리 이해 하라고 적은것. 간단히 WVP 곱하고 역행렬 취하면 됨


}

void Viewport::Project(OUT Vector3 * position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{
	Matrix wvp = W * V*P;
	Vector3 temp = source;
	D3DXVec3TransformCoord(position, &temp, &wvp);
	//뷰포트의 x값
	position->x = ((position->x + 1.0f) *0.5f)*width + x;
	position->y = ((-position->y + 1.0f) *0.5f)*height + y;
	//없어도 되긴함? 정규화 되서 0~1로
	position->z = (position->z * (maxDepth - minDepth)) + minDepth;
}

void Viewport::Unproject(OUT Vector3 * position, Vector3 & source, Matrix & W, Matrix & V, Matrix & P)
{	
	Vector3 temp = source;
	//뷰포트의 x값
	position->x = ((temp.x - x) / width)*2.0f - 1.0f;
	position->y = (((temp.y - y) / height)*2.0f - 1.0f) * -1.0f;
	//없어도 되긴함? 정규화 되서 0~1로
	position->z = (temp.z - minDepth) / (maxDepth - minDepth);

	Matrix wvp = W * V*P;
	D3DXMatrixInverse(&wvp, NULL, &wvp);
		
	//무조건 카메라 방향으로 쏴주기 때문에 화면에서 안보임?
	D3DXVec3TransformCoord(position, position, &wvp);


}


void Viewport::RSSetViewport()
{
	D3D::GetDC()->RSSetViewports(1, &viewport);
}