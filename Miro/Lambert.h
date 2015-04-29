#ifndef CSE168_LAMBERT_H_INCLUDED
#define CSE168_LAMBERT_H_INCLUDED

#include "Material.h"

class Lambert : public Material
{
public:
    Lambert(const Vector3 & kd = Vector3(1),
            const Vector3 & ka = Vector3(0));
	Lambert(const Vector3 & kd, const Vector3 & ka, float noise, float reflection);
    virtual ~Lambert();

    const Vector3 & kd() const {return m_kd;}
    const Vector3 & ka() const {return m_ka;}

    void setKd(const Vector3 & kd) {m_kd = kd;}
    void setKa(const Vector3 & ka) {m_ka = ka;}

    virtual void preCalc() {}
    
    virtual Vector3 shade(const Ray& ray, const HitInfo& hit,
                          const Scene& scene) const;
protected:
    Vector3 m_kd;
	float noise;
	float reflection;
    Vector3 m_ka;
    float m_ks = 0.7; //Specular reflectance radiance
    float m_kt; //Specular refracture radiance
    bool reflective = true;
};

#endif // CSE168_LAMBERT_H_INCLUDED
