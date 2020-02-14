#pragma once
#include "Framework.h"

struct ParticleData final
{
	ParticleData() { ZeroMemory(this, sizeof(ParticleData)); }
	std::string Name;

	float Duration;
	float ParticleLifespan;
	float ParticleLifespanVariance;

	float Speed;
	float SpeedVariance;

	Vector3 SourcePosition;
	Vector3 SourcePositionVariance;

	float StartParticleSize;
	float StartParticleSizeVariance;

	float FinishParticleSize;
	float FinishParticleSizeVariance;

	float Angle;
	float AngleVariance;

	float StartRotation;
	float StartRotationVariance;

	float FinishRotation;
	float FinishRotationVariance;

	float RotationPerSecond;
	float RotationPerSecondVariance;

	float MinRadius;
	float MinRadiusVariance;

	float MaxRadius;
	float MaxRadiusVariance;

	float RadialAcceleration;
	float RadialAccelVariance;

	float TangentialAcceleration;
	float TangentialAccelVariance;

	Vector3 Force;

	Color StartColor;
	Color StartColorVariance;

	Color FinishColor;
	Color FinishColorVariance;

	float EmissionRate;
};