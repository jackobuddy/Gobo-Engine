#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>

// TODO: This is a work-in-progress with the hope to eventually decouple oursellves from
// the GLM Math library. Lots of additions will be needed first. 

// ----------------------------------------------------------------------------
// A 2-Deminsional Vector that stores values x and y. 
// ----------------------------------------------------------------------------
struct Vector2
{
	float x = 0, y = 0; 

	Vector2() = default; 

	Vector2(float x, float y)
	{
		this->x = x; 
		this->y = y; 
	}

	explicit constexpr Vector2(float value)
	{
		this->x = value; 
		this->y = value; 
	}

	float& operator [](int i) { return ((&x)[i]); }
	const float& operator [](int i) const { return ((&x)[i]); }

	Vector2& operator *=(float s)
	{
		x *= s; 
		y *= s; 
		return (*this); 
	}

	Vector2& operator /=(float s)
	{
		s = 1.0f / s; 
		x *= s; 
		y *= s; 
		return (*this); 
	}

	Vector2& operator +=(const Vector2& v)
	{
		x += v.x; 
		y += v.y; 
		return (*this);
	}

	Vector2& operator -=(const Vector2& v)
	{
		x -= v.x; 
		y -= v.y; 
		return (*this); 
	}
};

inline Vector2 operator *(const Vector2& v, float s)
{
	return (Vector2(v.x * s, v.y * s)); 
}

inline Vector2 operator /(const Vector2& v, float s)
{
	s = 1.0f / s; 
	return Vector2(v.x * s, v.y * s); 
}

inline Vector2 operator -(const Vector2& v)
{
	return Vector2(-v.x, -v.y); 
}

inline float Magnitude(const Vector2& v)
{
	return sqrt(v.x * v.x + v.y * v.y); 
}

inline Vector2 Normalize(const Vector2& v)
{
	const float magnitude = Magnitude(v); 
	if (magnitude == 0.0f)
		return Vector2{}; 

	return v / magnitude; 
}

inline Vector2 operator +(const Vector2& a, const Vector2& b)
{
	return (Vector2(a.x + b.x, a.y + b.y));
}

inline Vector2 operator -(const Vector2& a, const Vector2& b)
{
	return (Vector2(a.x - b.x, a.y - b.y));
}

// ----------------------------------------------------------------------------
// A 3-Deminsional Vector that stores values x, y, and z. 
// ----------------------------------------------------------------------------
struct Vector3
{
	float x = 0, y = 0, z = 0;

	Vector3() = default;

	Vector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z; 
	}

	explicit constexpr Vector3(float value)
	{
		this->x = value;
		this->y = value;
		this->z = value; 
	}

	float& operator [](int i) 
	{ 
		assert(i < 2);
		return ((&x)[i]); 
	}

	const float& operator [](int i) const 
	{ 
		assert(i < 2);
		return ((&x)[i]); 
	}

	Vector3& operator *=(float s)
	{
		x *= s;
		y *= s;
		z *= s; 
		return (*this);
	}

	Vector3& operator /=(float s)
	{
		s = 1.0f / s;
		x *= s;
		y *= s;
		z *= s; 
		return (*this);
	}

	Vector3& operator +=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z; 
		return (*this);
	}

	Vector3& operator -=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z; 
		return (*this);
	}
};

inline Vector3 operator *(const Vector3& v, float s)
{
	return (Vector3(v.x * s, v.y * s, v.z * s));
}

inline Vector3 operator /(const Vector3& v, float s)
{
	s = 1.0f / s;
	return Vector3(v.x * s, v.y * s, v.z * s);
}

inline Vector3 operator -(const Vector3& v)
{
	return Vector3(-v.x, -v.y, -v.z);
}

inline float Magnitude(const Vector3& v)
{
	return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

inline Vector3 Normalize(const Vector3& v)
{
	const float magnitude = Magnitude(v);
	if (magnitude == 0.0f)
		return Vector3{};

	return v / magnitude;
}

inline Vector3 operator +(const Vector3& a, const Vector3& b)
{
	return (Vector3(a.x + b.x, a.y + b.y, a.z + b.z));
}

inline Vector3 operator -(const Vector3& a, const Vector3& b)
{
	return (Vector3(a.x - b.x, a.y - b.y, a.z - b.z));
}