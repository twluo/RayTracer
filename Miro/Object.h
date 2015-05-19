#ifndef CSE168_OBJECT_H_INCLUDED
#define CSE168_OBJECT_H_INCLUDED

#include <vector>
#include "Miro.h"
#include "Material.h"

class Object
{
public:
    Object() {}
    virtual ~Object() {}

    void setMaterial(const Material* m) {m_material = m;}

    virtual void renderGL() {}
    virtual void preCalc() {}
	virtual Vector3 getCenteroid() { return Vector3(0, 0, 0); }
	virtual Vector3 getMax() { return Vector3(0, 0, 0); }
	virtual Vector3 getMin() { return Vector3(0, 0, 0); }


    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX) = 0;
	bool hit;

protected:
    const Material* m_material;
};

typedef std::vector<Object*> Objects;

#endif // CSE168_OBJECT_H_INCLUDED
