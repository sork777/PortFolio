#pragma once
class Math
{
public:
	static const float PI;
	static const float EPSILON;

	static float Modulo(float val1, float val2);

	static float ToRadian(float degree);
	static float ToDegree(float radian);

	static Vector3 Cross(Vector3& vec1, Vector3& vec2);

	static int Random(int r1, int r2);
	static float Random(float r1, float r2);
	
	static Vector2 RandomVec2(float r1, float r2);
	static Vector3 RandomVec3(float r1, float r2);
	static D3DXCOLOR RandomColor3();
	static D3DXCOLOR RandomColor4();

	static float Clamp(float value, float min, float max);

	static void LerpMatrix(OUT Matrix& out, const Matrix& m1, const Matrix& m2, float amount);
	static void LerpMatrixSRT(OUT Matrix& out, const Matrix& m1, const Matrix& m2, float amount);

	static D3DXQUATERNION LookAt(const Vector3& origin, const Vector3& target, const Vector3& up);

	static void toEulerAngle(const D3DXQUATERNION& q, float& pitch, float& yaw, float& roll);
	static void toEulerAngle(const D3DXQUATERNION& q, Vector3& out);
	static void MatrixDecompose(const Matrix& m, OUT Vector3& S, OUT Vector3& R, OUT Vector3& T);

	static float Gaussian(const float& val, const UINT& blurCount);
	static float fABS(const float& val);
	
	template <typename T>
	static T Max(T a, T b);
	template <typename T>
	static T Min(T a, T b);
//
//public:
//	static Vector3 ClosestPtPointSegment(const Vector3& pt, const Vector3& segA, const Vector3& segB);
//	static void ClosestPtPointOBB(const Vector3 & pt, const Vector3 & obbCenter, const Vector3 obbAxis[3], const float obbSize[3], OUT Vector3 & q);
//	static void ClosestPtSegmentSegment(const Vector3 & segAS, const Vector3 & segAE, const Vector3 & segBS, const Vector3 & segBE, OUT Vector3 & c1, OUT Vector3 & c2);

};

template<typename T>
inline T Math::Max(T a, T b)
{
	return T(a>b?a:b);
}

template<typename T>
inline T Math::Min(T a, T b)
{
	return T(a < b ? a : b);
}
