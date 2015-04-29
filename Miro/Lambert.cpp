#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "Worley.h"
#include <algorithm>
#include "Perlin.h"

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka) :
m_kd(kd), m_ka(ka), noise(0)
{

}
Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, int noise) :
m_kd(kd), m_ka(ka), noise(noise)
{

}

Lambert::~Lambert()
{
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);

	const Vector3 viewDir = -ray.d; // d is a unit vector

	const Lights *lightlist = scene.lights();

	const int maxOrder = 4;
	float F[maxOrder];
	float at[3] = { hit.P.x, hit.P.y, hit.P.z };
	float delta[maxOrder][3];
	unsigned long *ID = new unsigned long();

	WorleyNoise::noise3D(at, maxOrder, F, delta, ID);
	L += (F[2] - F[1]) * noise;
	L += PerlinNoise::noise(hit.P.x, hit.P.y, hit.P.z) * noise;

	// loop over all of the lights
	Lights::const_iterator lightIter;
	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
	{
		PointLight* pLight = *lightIter;

		Vector3 l = pLight->position() - hit.P;

		// the inverse-squared falloff
		float falloff = l.length2();

		// normalize the light direction
		l /= sqrt(falloff);

		// get the diffuse component
		float nDotL = dot(hit.N, l);
		Vector3 result = pLight->color();
		result *= m_kd;

		L += std::max(0.0f, nDotL / falloff * pLight->wattage() / PI) * result;
	}
	// add the ambient component
	L += m_ka;
	//L += F[0];
    
    return L;
}
