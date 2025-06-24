#pragma once

struct Quaternion
{
	float x, y, z, w;
};

inline Quaternion operator/(const Quaternion& q1, float& n) {
	Quaternion result;

	result.w = q1.w / n;
	result.x = q1.x / n;
	result.y = q1.y / n;
	result.z = q1.z / n;
	return result;
}