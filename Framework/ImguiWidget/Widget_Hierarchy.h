#pragma once
#include "IWidget.h"

class Widget_Hierarchy :
	public IWidget
{
public:
	Widget_Hierarchy();
	~Widget_Hierarchy();

	// IWidget��(��) ���� ��ӵ�
	virtual void Render() override;
};

