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
