#ifndef KOI_VECTOR_H
#define KOI_VECTOR_H

#include <unordered_set>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

typedef glm::vec2 Vec2;
typedef glm::vec3 Vec3;
typedef glm::vec4 Vec4;
typedef glm::mat4 Mat4;

/*struct Vec4
{
	float x;
	float y;
	float z;
	float w;

	float& operator [] (const int i)
	{
		return *((float *) (this) + i);
	}

	bool operator == (const Vec4& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}
};

struct Vec3
{
	float x;
	float y;
	float z;

	float& operator [] (const int i)
	{
		return *((float *) (this) + i);
	}

	bool operator == (const Vec3& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	operator Vec4 () const
	{
		Vec4 result = {x, y, z, 1.0f};
		return result;
	}
};

struct Vec2
{
	float x;
	float y;

	float& operator [] (const int i)
	{
		return *((float *) (this) + i);
	}

	bool operator == (const Vec2& other) const
	{
		return x == other.x && y == other.y;
	}

	operator Vec3 () const
	{
		Vec3 result = {x, y, 0.0};
		return result;
	}
};

struct Mat4
{
	Vec4 a;
	Vec4 b;
	Vec4 c;
	Vec4 d;

	Vec4& operator [] (const int i)
	{
		return *((Vec4 *) (this) + i);
	}
};*/

namespace std
{
	inline void hashy(size_t& seed, size_t hash)
	{
		hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
		seed ^= hash;
	}

	template<> struct hash<Vec2>
	{
	    size_t operator()(Vec2 const& vec2) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec2.x));
			hashy(seed, hasher(vec2.y));
			return seed;
	    }
	};

	template<> struct hash<Vec3>
	{
	    size_t operator()(Vec3 const& vec3) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec3.x));
			hashy(seed, hasher(vec3.y));
			hashy(seed, hasher(vec3.z));
			return seed;
	    }
	};

	template<> struct hash<Vec4>
	{
	    size_t operator()(Vec4 const& vec4) const
		{
			size_t seed = 0;
			hash<float> hasher;
			hashy(seed, hasher(vec4.x));
			hashy(seed, hasher(vec4.y));
			hashy(seed, hasher(vec4.z));
			hashy(seed, hasher(vec4.w));
			return seed;
	    }
	};
}

#endif