#ifndef CSE168_RAY_H_INCLUDED
#define CSE168_RAY_H_INCLUDED

#include "Vector3.h"

class Ray
{
public:
    Vector3 o,      //!< Origin of ray
        d;      //!< Direction of ray
    int times;
	float snell;
	int RBIntersections;
	int RLIntersections;
	float power;
	Vector3 invd;
	Vector3 dirs;
	Vector3 inv() {
		return Vector3(1 / d.x, 1 / d.y, 1 / d.z);
	}
	void update() {
		invd = inv();
	}

	Ray() : o(), d(Vector3(0.0f, 0.0f, 1.0f)), times(0), snell(1)
    {
		invd = inv();
		//std::cout << d << std::endl;
		//std::cout << dirs << std::endl;
    }

	Ray(const Vector3& o, const Vector3& d) : o(o), d(d), times(0), snell(1)
	{
		invd = inv();
		//std::cout << d << std::endl;
		//std::cout << dirs << std::endl;
        // empty
    }
};


//! Contains information about a ray hit with a surface.
/*!
HitInfos are used by object intersection routines. They are useful in
order to return more than just the hit distance.
*/
class HitInfo
{
public:
    float t;                            //!< The hit distance
    Vector3 P;                          //!< The hit point
    Vector3 N;                          //!< Shading normal vector
    const Material* material;           //!< Material of the intersected object

    //! Default constructor.
    explicit HitInfo(float t = 0.0f,
        const Vector3& P = Vector3(),
        const Vector3& N = Vector3(0.0f, 1.0f, 0.0f)) :
        t(t), P(P), N(N), material(0)
    {
        // empty
    }
};

#endif // CSE168_RAY_H_INCLUDED