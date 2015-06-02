#include "Lambert.h"
#include "Ray.h"
#include "Scene.h"
#include "Worley.h"
#include <algorithm>
#include "Perlin.h"
#include <ctime>

Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const Vector3 &ks) 
{
	m_kd = kd;
	m_ka = ka;
	m_ks = ks;
	rd = 1;
	ra = 0;
	rs = 0;
	noise = 0;
	reflection = 0;
	refractive = 0;
	refra = false;
	refle = false;
	snell = 1;
}
Lambert::Lambert(const Vector3 & kd, const Vector3 & ka, const Vector3 & ks,
	float rd, float ra, float rs, 
	float noise, float reflection, float refractive, float snell) 
{
	m_kd = kd;
	m_ka = ka;
	m_ks = ks;
	this->rd = rd;
	this->ra = ra;
	this->rs = rs;
	this->noise = noise;
	this->reflection = reflection;
	this->refractive = refractive;
	this->snell = snell;

}

Lambert::~Lambert()
{
}


void Lambert::setAmbient(const Vector3 &ka, float ra)  {
	this->ra = ra;
	this->m_ka = ka;
}
void Lambert::setDiffuse(const Vector3 &kd, float rd) {
	this->rd = rd;
	this->m_kd = kd;
}
void Lambert::setSpecular(const Vector3 &ks, float rs) {
	this->rs = rs;
	this->m_ks = ks;
}
void Lambert::setReflectionConst(float rf) {
	this->reflection = rf;
	refle = true;
}
void Lambert::setRefractionConst(float rf) {
	this->refractive = rf;
	refra = true;
}
void Lambert::setSnellConstant(float snell) {
	this->snell = snell;
}
void Lambert::setPattern(float noise) {
	this->noise = noise;
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
	L += calcMonteCarlo(hit, ray, scene);
	L += calcRefraction(hit, ray, scene);
    return L;
}


Vector3 hemisphereSample_cos(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}
Vector3 Lambert::calcMonteCarlo(HitInfo hit, Ray ray, Scene scene) const{
	Ray r;
	HitInfo hi; 
	const Lights *lightlist = scene.lights();
	// TODO:: MULTIPLY BY SPECULAR
	if (ray.times == 1)
		return Vector3(0);
	else if (ray.times < 1) {
		// transform x and y into camera space 
		// -----------------------------------
		//srand(time(NULL));
		float v = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		float w = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		Vector3 dir = hemisphereSample_cos(v, w);
		
		float rx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		float ry = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		float rz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
		Vector3 rand = Vector3(rx,ry,rz);
		rand.normalize();
		Vector3 n = hit.N;
		Vector3 t = cross(rand,n);
		Vector3 s = cross(n,t);
		r.o = hit.P;
		r.d = dir.x * t + dir.z * n + dir.y * s;
		r.d.normalize();
		r.update();
		r.times = ray.times + 1;
		if (scene.trace(hi, r)) {
			return rd * dot(r.d, hit.N) * hi.material->shade(r, hi, scene);
		}
		return Vector3(0);
	}
}

Vector3 Lambert::calcRefraction(HitInfo hit, Ray ray, Scene scene) const{
	Vector3 L;
	Ray r;
	HitInfo hi;
	const Vector3 viewDir = -ray.d; // d is a unit vector
	if (refra) {
		if (ray.times < 9) {
			r.o = hit.P;
			float dot_ = dot(viewDir, hit.N);
			float ratio;
			ratio = ray.snell / this->snell;
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
				r.o = r.o + r.d * epsilon;
				r.d.normalize();
				r.update();
				r.times = ray.times + 1;
				if (scene.trace(hi, r)) {
					if (hi.t > epsilon)
						L += refractive * hi.material->shade(r, hi, scene);
				}
			}
	       }
	}
	return L;
}
Vector3 Lambert::calcReflection(HitInfo hit, Ray ray, Scene scene) const{
	Vector3 L;
	Ray r;
	HitInfo hi;
	if (refle) {
		// TODO:: MULTIPLY BY SPECULAR
		if (ray.times < 2) {
			r.o = hit.P;
			r.d = -2 * dot(ray.d, hit.N)*hit.N + ray.d;
			r.o = r.o + r.d * epsilon;
			r.d.normalize();
			r.update();
			r.times = ray.times + 1;
			if (scene.trace(hi, r)) {
				if (hi.t > epsilon)
					L += reflection * hi.material->shade(r, hi, scene);
			}
		}
	}
	return L;
}

//TODO: FIX THIS SHIT!
/*if (noise != 0) {
Vector3 patterC;
const int maxOrder = 3;
float F[maxOrder];
float at[3] = { hit.P.x, hit.P.y, hit.P.z };
float delta[maxOrder][3];
unsigned long *ID = new unsigned long();

WorleyNoise::noise3D(at, maxOrder, F, delta, ID);
patterC  = F[2] - F[1];
L += rd * patterC + PerlinNoise::noise(at[0], at[1], at[2]) * noise;
}
else {*/