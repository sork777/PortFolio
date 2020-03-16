#include "Framework.h"
#include "Collider.h"



Collider::Collider(Transform * transform, Transform * init)
	:transform(transform), init(init), SelPos(-1, -1, -1)
	, bUsingCollider(true)
{	

}


Collider::~Collider()
{
}

Vector3 Collider::Cross(Vector3 & vec1, Vector3 & vec2)
{
	float x = vec1.y*vec2.z - vec1.z*vec2.y;
	float y = vec1.z*vec2.x - vec1.x*vec2.z;
	float z = vec1.x*vec2.y - vec1.y*vec2.x;

	return Vector3(x, y, z);
}
