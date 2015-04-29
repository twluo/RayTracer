#ifndef CSE168_MATERIAL_H_INCLUDED
#define CSE168_MATERIAL_H_INCLUDED

#include "Miro.h"
#include "Vector3.h"

class Material
{
public:
    Material();
    virtual ~Material();

    virtual void preCalc() {}

    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
        const Scene& scene) const;

protected:
    float rd; //Diffuse radiance
    float rs; //Specular reflectance radiance
    float rt; //Specular refracture radiance
};

#endif // CSE168_MATERIAL_H_INCLUDED