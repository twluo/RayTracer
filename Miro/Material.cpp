#include "Material.h"

Material::Material()
{
}

Material::~Material()
{
}

Vector3
Material::shade(const Ray& r, const HitInfo& h, const Scene& s) const
{
    
    
    return Vector3(1.0f, 1.0f, 1.0f);
}

Vector3
Material::photonShade(const Ray& r, const HitInfo& h, const Scene& s, const Photon_map &pmap) const
{


    return Vector3(1.0f, 1.0f, 1.0f);
}
