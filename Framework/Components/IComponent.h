#pragma once
#include "Framework.h"


__interface IComponent
{
public:
	virtual void Update() = 0;
	virtual void Render() = 0;
};
