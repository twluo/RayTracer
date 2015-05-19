#ifndef CSE168_BVH_H_INCLUDED
#define CSE168_BVH_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include "box.h"

class BVH
{
public:
    void build(Objects * objs);
	void draw();
    bool intersect(HitInfo& result, const Ray& ray, 
                   float tMin = 0.0f, float tMax = MIRO_TMAX);
	box getBox() { return bbox; }
	void count(int &leafCount, int &nodeCount);
	void countIntersections(int &RBI, int &RLI);
	int RB;
	int RL;

protected:
	bool leaf;
    Objects * m_objects;
	BVH* leftBox;
	BVH* rightBox;
	box bbox;
};

#endif // CSE168_BVH_H_INCLUDED
