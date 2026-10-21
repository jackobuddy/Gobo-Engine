#pragma once

#include <cassert>
#include <cstddef>
#include <cmath>
#include <cstdint>
#include <chrono>
#include <limits>
#include <random>
#include <stdexcept>

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

	float& operator [](int i) 
	{ 
		assert(i >= 0 && i < 2);
		return ((&x)[i]); 
	}
	const float& operator [](int i) const 
	{ 
		assert(i >= 0 && i < 2);
		return ((&x)[i]); 
	}

	Vector2& operator *=(float s)
	{
		x *= s; 
		y *= s; 
		return (*this); 
	}

	Vector2& operator /=(float s)
	{
		if (s == 0.0f)
			return (*this);
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
	if (s == 0.0f)
		return Vector2{};
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

inline float Dot(const Vector2& a, const Vector2& b)
{
	return (a.x * b.x + a.y * b.y);
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
		assert(i >= 0 && i < 3);
		return ((&x)[i]); 
	}

	const float& operator [](int i) const 
	{ 
		assert(i >= 0 && i < 3);
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
		if (s == 0.0f)
			return (*this);
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
	if (s == 0.0f)
		return Vector3{};
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

inline float Dot(const Vector3& a, const Vector3& b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

// ----------------------------------------------------------------------------
// Random Functions
// ----------------------------------------------------------------------------

namespace GoboRandomDetail
{
	inline std::mt19937& Generator()
	{
		thread_local std::mt19937 generator = [] {
			std::random_device device;
			const auto ticks = static_cast<std::uint64_t>(
				std::chrono::steady_clock::now().time_since_epoch().count());

			std::seed_seq seed{
				device(), device(), device(), device(),
				static_cast<std::uint32_t>(ticks),
				static_cast<std::uint32_t>(ticks >> 32)
			};
			return std::mt19937(seed);
			}();

		return generator;
	}
}

inline int random_range(int min, int max)
{
	if (min > max)
		throw std::invalid_argument("random_range: min exceeds max");

	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(GoboRandomDetail::Generator());
}

inline float random_range(float min, float max)
{
	if (!std::isfinite(min) || !std::isfinite(max) || min > max)
		throw std::invalid_argument("random_range: invalid float range");

	if (min == max)
		return min;

	// Include both endpoints. uniform_real_distribution excludes its upper bound.
	constexpr std::uint32_t steps = 1u << 24;
	std::uniform_int_distribution<std::uint32_t> distribution(0u, steps);
	const double fraction =
		static_cast<double>(distribution(GoboRandomDetail::Generator())) /
		static_cast<double>(steps);

	return static_cast<float>(
		std::lerp(static_cast<double>(min),
			static_cast<double>(max),
			fraction));
}

inline bool random_flip()
{
	std::bernoulli_distribution distribution(0.5);
	return distribution(GoboRandomDetail::Generator());
}

// ----------------------------------------------------------------------------
// Other Math functions. 
// ----------------------------------------------------------------------------
inline float Clamp(float a, float min, float max)
{
	if (a < min) a = min; 
	if (a > max) a = max; 
	return a; 
}

inline int Clamp(int a, int min, int max)
{
	if (a < min) a = min; 
	if (a > max) a = max; 
	return a; 
}

inline std::uint64_t Clamp(std::uint64_t a, std::uint64_t min, std::uint64_t max)
{
	if (a < min) a = min;
	if (a > max) a = max;
	return a;
}

inline float Floor(float value)
{
	return std::floor(value);
}

inline int FloorToInt(float value)
{
	return static_cast<int>(std::floor(value));
}

// A set of Red / Green / Blue / and Alpha values between 0 and 255. 
struct Color
{
	int r;
	int g;
	int b;
	int a;

	Color(int r = 0, int g = 0, int b = 0, int alpha = 0)
	{
		this->r = Clamp(r, 0, 255);
		this->g = Clamp(g, 0, 255);
		this->b = Clamp(b, 0, 255);
		this->a = Clamp(alpha, 0, 255);
	}
};