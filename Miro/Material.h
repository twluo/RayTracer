#ifndef CSE168_MATERIAL_H_INCLUDED
#define CSE168_MATERIAL_H_INCLUDED

#include "Miro.h"
#include "Vector3.h"
#include "photonMap.h"

class Material
{
public:
    Material();
    virtual ~Material();

	virtual void preCalc() {}
	virtual void setAmbient(const Vector3 &ka, float ra) {}
	virtual void setDiffuse(const Vector3 &kd, float rd) {}
	virtual void setSpecular(const Vector3 &ks, float r) {}
	virtual void setConstant(float rd, float rs, float rf) {}
	virtual void setReflectionConst(float rf) {}
	virtual void setRefractionConst(float rf) {}
	virtual void setSnellConstant(float snell) {}
	virtual void setPattern(float noise) {}

    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
		const Scene& scene) const;
    virtual Vector3 photonShade(const Ray& ray, const HitInfo& hit,
        const Scene& scene, const Photon_map &pmap) const;
	float snell;
	float rd; //Diffuse radiance
	float rs; //Specular reflectance radiance
	float rf; //Specular refracture radiance
    Vector3 m_kd;
    Vector3 m_ks;
	Vector3 m_ka;

protected:
};

#endif // CSE168_MATERIAL_H_INCLUDED