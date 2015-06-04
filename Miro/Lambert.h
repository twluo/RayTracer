#ifndef CSE168_LAMBERT_H_INCLUDED
#define CSE168_LAMBERT_H_INCLUDED

#include "Material.h"

class Lambert : public Material
{
public:
    Lambert(const Vector3 & kd = Vector3(1),
        const Vector3 & ka = Vector3(0),
		const Vector3 & ks = Vector3(0));
    Lambert(const Vector3 & kd, const Vector3 & ka, const Vector3 & ks,
		float rd, float ra, float rs, 
		float noise, float reflection, float refractive, float snell);
    virtual ~Lambert();

	virtual void setAmbient(const Vector3 &ka, float ra);
	virtual void setDiffuse(const Vector3 &kd, float rd);
	virtual void setSpecular(const Vector3 &ks, float r);
	virtual void setReflectionConst(float rf);
	virtual void setRefractionConst(float rf);
	virtual void setSnellConstant(float snell);
	virtual void setPattern(float noise);
	virtual void setConstant(float rd, float rs, float rf);
	Vector3 calcReflection(const Ray &ray, const HitInfo &hit, Scene scene) const;
	Vector3 calcRefraction(const Ray &ray, const HitInfo &hit, Scene scene) const;
	Vector3 calcMonteCarlo(const Ray &ray, const HitInfo &hit, Scene scene) const;
	Vector3 calcPhotonReflection(const Ray &ray, const HitInfo &hit, Scene scene,
		const Photon_map& pmap) const;
    const Vector3 & kd() const { return m_kd; }
	const Vector3 & ka() const { return m_ka; }
	float snell;

    void setKd(const Vector3 & kd) { m_kd = kd; }
    void setKa(const Vector3 & ka) { m_ka = ka; }

    virtual void preCalc() {}

    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
		const Scene& scene) const;
    virtual Vector3 photonShade(const Ray& ray, const HitInfo& hit,
        const Scene& scene, const Photon_map& pmap) const;
	float rd; //Diffuse radiance
	float rs; //Specular reflectance radiance
	float rf;//Specular refracture radiance

    Vector3 m_kd;
    Vector3 m_ks;
protected:
    Vector3 m_ka;
    float noise;
	float ra;
	bool refra;
	bool refle;
};

#endif // CSE168_LAMBERT_H_INCLUDED