#ifndef CSE168_BOX_H_INCLUDED
#define CSE168_BOX_H_INCLUDED

#include "Object.h"
#include "Ray.h"
class box
{
public:

	box();
	box(Vector3, Vector3);
	~box();
	bool intersect(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const;
	void print();
	void draw();

protected:

	Vector3 min;
	Vector3 max;
};

#endif