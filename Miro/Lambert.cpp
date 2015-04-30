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
	float noise, float reflection, float refractive, float snell) :
m_kd(kd), m_ka(ka), m_ks(ks),
rd(rd), ra(ra), rs(rs), 
noise(noise), reflection(reflection), refractive(refractive), snell(snell)
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
	bool shadow = true;
	const Vector3 viewDir = -ray.d; // d is a unit vector

	const Lights *lightlist = scene.lights();
	Vector3 cellN = Vector3(1);
	float cN = 1;


	// loop over all of the lights
	Lights::const_iterator lightIter;
	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
	{
		PointLight* pLight = *lightIter;

		Vector3 l = pLight->position() - hit.P;

		Vector3 W_r = 2 * dot(l, hit.N)*hit.N - l;
		W_r.normalize();

		// the inverse-squared falloff
		float falloff = l.length2();

		// normalize the light direction
		l /= sqrt(falloff);

		// get the diffuse component
		float nDotL = dot(hit.N, l);
		Vector3 color = pLight->color();
		L += std::max(0.0f, nDotL / falloff * pLight->wattage() / PI) * rd * color * m_kd;
		L += std::max(0.0f, powf(dot(viewDir, W_r), 1000)) * rs * color * m_ks;
	}

	L += m_ka * ra;
	// add the ambient component
	if (reflection != 0) {
		if (ray.times < 2) {
			r.o = hit.P;
			r.d = -2 * dot(ray.d, hit.N)*hit.N + ray.d;
			r.d.normalize();
			r.o = r.o + r.d * epsilon;
			r.times = ray.times + 1;
			if (scene.trace(hi, r)) {
				if (hi.t > epsilon)
					L += reflection * hi.material->shade(r, hi, scene);
			}
		}
	}
    if (refractive != 0) {
        if (ray.times < 6) {
            r.o = hit.P;
			float dot_ = dot(viewDir, hit.N);
			float ratio;
			ratio = r.snell / this->snell;
			if (dot_ > 0.0f) {
				r.snell = this->snell;
			}
			else {
				r.snell = 1.0;
				dot_ = -dot_;
			}
			float temp = (1 - pow(ratio, 2) * (1 - pow(dot_, 2)));
			if (temp >= 0) {
				r.d = -1 * ratio * (viewDir - dot_ * hit.N) - sqrt(temp)  * hit.N;
				r.d.normalize();
				r.o = r.o + r.d * epsilon;
				r.times = ray.times + 1;
				if (scene.trace(hi, r)) {
					if (hi.t > epsilon)
						L += refractive * hi.material->shade(r, hi, scene);
				}
			}
        }
	}
	if (noise != 0) {
		const int maxOrder = 3;
		float F[maxOrder];
		float at[3] = { hit.P.x, hit.P.y, hit.P.z };
		float delta[maxOrder][3];
		unsigned long *ID = new unsigned long();

		WorleyNoise::noise3D(at, maxOrder, F, delta, ID);
		
		//cellN.x = delta[1][0] * noise;
		//cellN.y = delta[1][1] * noise;
		//cellN.z = delta[1][2] * noise;
		
		cN = (F[2] - F[0]);
		L += cN;
		//L += cellN ;
		L += PerlinNoise::noise(delta[0][0] + at[0], delta[0][1] + at[1], delta[0][2] + at[2]) ;
	}
 
    return L;
}
