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
	virtual void setAmbient(const Vector3 &ka, float ra) {}
	virtual void setDiffuse(const Vector3 &kd, float rd) {}
	virtual void setSpecular(const Vector3 &ks, float r) {}
	virtual void setReflectionConst(float rf) {}
	virtual void setRefractionConst(float rf) {}
	virtual void setSnellConstant(float snell) {}
	virtual void setPattern(float noise) {}

    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
        const Scene& scene) const;

protected:
    float rd; //Diffuse radiance
    float rs; //Specular reflectance radiance
    float rt; //Specular refracture radiance
};

#endif // CSE168_MATERIAL_H_INCLUDED