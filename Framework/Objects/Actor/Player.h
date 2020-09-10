#pragma once
#include "Actor.h"

//... 상속과정에서 이클래스의 헤더에 문제가 있었음 0901
class Player :
	public Actor
{
public:
	Player();
	~Player();
};

