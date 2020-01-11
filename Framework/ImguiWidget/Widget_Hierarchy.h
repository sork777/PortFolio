#pragma once
#include "IWidget.h"

class Widget_Hierarchy :
	public IWidget
{
public:
	Widget_Hierarchy();
	~Widget_Hierarchy();

	// IWidget을(를) 통해 상속됨
	virtual void Render() override;
};

