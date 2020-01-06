#pragma once


class Freedom : public Camera
{
public:
	Freedom();
	virtual ~Freedom();

	void Update() override;

	void Speed(float move, float rot);

	void Movable(bool bMove) { this->bMove = bMove; }

private:
	float move;
	float rot;
	bool bMove;

};

