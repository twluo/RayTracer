#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "Worley.h"
#include <algorithm>
#include "Perlin.h"

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka) :
m_kd(kd), m_ka(ka), noise(0), reflection(0)
{

}
Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, float noise, float reflection, float refractive) :
m_kd(kd), m_ka(ka), noise(noise), reflection(reflection), refractive(refractive)
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
		float at[3] = { hit.P.x, hit.P.y, hit..z };
		float delta[maxOrder][3];
		unsigned long *ID = new unsigned long();

		WorleyNoise::noise3D(at, maxOrder, F, delta, ID);/*
		cellN.x = (delta[2][0] - delta[1][0]) * noise;
		cellN.y = (delta[2][1] - delta[1][1]) * noise;
		cellN.z = (delta[2][2] - delta[1][2]) * noise;*/
		cellN = (F[2] - F[1]) * noise;
		L += PerlinNoise::noise(at[0], at[1], at[2]) * noise;
	}

    Vector3 W_r = -2 * (dot(viewDir, hit.N))*hit.N + viewDir;

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
		result *= m_kd * cellN;

		L += std::max(0.0f, nDotL / falloff * pLight->wattage() / PI) * result;
        float temp;
        if ((dot(viewDir, W_r)) > 0){
            temp = (dot(viewDir, W_r));
        }
        else{
            temp = 0;
        }

        Vector3 L_phong = rs*pow((temp), 0.2);// *abs(nDotL);
        L += L_phong;
	}
	// add the ambient component
	L += m_ka; 
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
    else if (refractive != 0) {
        if (ray.times < 3) {
            r.o = hit.P;
            r.d = -1 * (eta1 / eta2) * (viewDir - dot(viewDir, hit.N) * hit.N) - sqrtf(1 - pow((eta1 / eta2), 2) * (1 - pow(dot(viewDir, hit.N), 2))) * hit.N;
            //r.d = -(eta1 / eta2)*(ray.d - dot(ray.d, hit.N)*hit.N) - sqrt(1 - pow((eta1 / eta2), 2) * (1 - pow(dot(hit.P, hit.N), 2)))*hit.N;
            r.times = ray.times + 1;
            if (scene.trace(hi, r)) {
                if (hi.t > epsilon)
                    L = reflection * hi.material->shade(r, hi, scene);
            }
        }
    }
	//L += F[0];
 
    return L;
}
