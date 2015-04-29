#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "Worley.h"
#include <algorithm>
#include "Perlin.h"

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const Vector3 &ks) :
m_kd(kd), m_ka(ka), m_ks(ks),
rd(0), ra(0), rs(0),
noise(0), reflection(0), refractive(refractive)
{

}
Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const Vector3 & ks,
	float rd, float ra, float rs, 
	float noise, float reflection, float refractive) :
m_kd(kd), m_ka(ka), m_ks(ks),
rd(rd), ra(ra), rs(rs), 
noise(noise), reflection(reflection), refractive(refractive)
{

}

Lambert::~Lambert()
{
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
	Ray r;
	HitInfo hi;
	const Vector3 viewDir = -ray.d; // d is a unit vector

	const Lights *lightlist = scene.lights();
	Vector3 cellN = Vector3(1);

	if (noise != 0) {
		const int maxOrder = 3;
		float F[maxOrder];
		float at[3] = { hit.P.x, hit.P.y, hit.P.z };
		float delta[maxOrder][3];
		unsigned long *ID = new unsigned long();

		WorleyNoise::noise3D(at, maxOrder, F, delta, ID);/*
		cellN.x = (delta[2][0] - delta[1][0]) * noise;
		cellN.y = (delta[2][1] - delta[1][1]) * noise;
		cellN.z = (delta[2][2] - delta[1][2]) * noise;*/
		cellN = (F[2] - F[1]) * noise;
		L += PerlinNoise::noise(at[0], at[1], at[2]) * noise;
	}


	// loop over all of the lights
	Lights::const_iterator lightIter;
	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
	{
		PointLight* pLight = *lightIter;

		Vector3 l = pLight->position() - hit.P;

		Vector3 W_r = (viewDir + l);
		W_r.normalize();

		// the inverse-squared falloff
		float falloff = l.length2();

		// normalize the light direction
		l /= sqrt(falloff);

		// get the diffuse component
		float nDotL = dot(hit.N, l);
		Vector3 color = pLight->color();
		Vector3 result = color * m_kd * cellN * rd;
		L += std::max(0.0f, nDotL / falloff * pLight->wattage() / PI) * result;
		L += std::max(0.0f, powf(dot(hit.N, W_r), 1000)) * rs * color;
	}

	L += m_ka * ra;
	// add the ambient component
	if (reflection != 0) {
		if (ray.times < 3) {
			r.o = hit.P;
			r.d = -2 * dot(ray.d, hit.N)*hit.N + ray.d;
			r.times = ray.times + 1;
			if (scene.trace(hi, r)) {
				if (hi.t > epsilon)
					L += reflection * hi.material->shade(r, hi, scene);
			}
		}
	}
    if (refractive != 0) {
        if (ray.times < 3) {
            r.o = hit.P;
            r.d = -1 * (eta1 / eta2) * (viewDir - dot(viewDir, hit.N) * hit.N) - sqrtf(1 - pow((eta1 / eta2), 2) * (1 - pow(dot(viewDir, hit.N), 2))) * hit.N;
            //r.d = -(eta1 / eta2)*(ray.d - dot(ray.d, hit.N)*hit.N) - sqrt(1 - pow((eta1 / eta2), 2) * (1 - pow(dot(hit.P, hit.N), 2)))*hit.N;
            r.times = ray.times + 1;
            if (scene.trace(hi, r)) {
                if (hi.t > epsilon)
                    L += reflection * hi.material->shade(r, hi, scene);
            }
        }
    }
	//L += F[0];
 
    return L;
}
