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
	rf = 0;
	trans = false;
	refle = false;
	refra = false;
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
	this->rf = refractive;
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

void Lambert::setTransmission(float rf, float snell) {
	this->rf = rf;
	trans = true;
	this->snell = snell;
}
void Lambert::setPattern(float noise) {
	this->noise = noise;
}

void Lambert::setRefle() {
	refle = true;
}

void Lambert::setRefra(float rf, float snell) {
	this->rf = rf;
	this->snell = snell;
	refra = true;
}
void Lambert::setConstant(float rd, float rs, float rf) {
		this->rd = rd;
		this->rs = rs;
		this->rf = rf;
}

Vector3
Lambert::shade(const Ray& ray, const HitInfo& hit, const Scene& scene) const
{
	Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
	const Vector3 viewDir = -ray.d; // d is a unit vector

	const Lights *lightlist = scene.lights();

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
	Ray r = ray;
	HitInfo hi = hit;
	Scene sc = scene; 
	if (refra)
		L += calcRefraction(ray, hit, scene);
	if (refle)
		L += calcReflection(ray, hit, scene);
	//L += calcMonteCarlo(r, hi, sc);
    return L;
}


Vector3 hemisphereSample_cos(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}

Vector3 Lambert::calcMonteCarlo(const Ray &ray, const HitInfo &hit, Scene scene) const{
	Ray r;
	HitInfo hi; 
	// TODO:: MULTIPLY BY SPECULAR
	if (ray.times >= 1)
		return Vector3(0);
	else if (ray.times < 1) {
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
			if (dot(r.d, hit.N) >= 0)
				return m_kd * hi.material->shade(r, hi, scene);
		}
		return Vector3(0);
	}
}

Vector3 Lambert::calcPhotonMonteCarlo(const Ray &ray, const HitInfo &hit, Scene scene,
 const Photon_map &pmap) const{

	float *dir = new float[3];
	float pos[3] = { hit.P.x, hit.P.y, hit.P.z };
	float norm[3] = { hit.N.x, hit.N.y, hit.N.z };
	pmap.dir_estimate(dir, pos, norm, .5, 500);
	Vector3 d = Vector3(dir[0], dir[1], dir[2]);
	Ray r;
	r.o = hit.P;
	r.d = -d;
	HitInfo hi;
	if (scene.trace(hi, r)) {
		float *ir = new float[3];
		float npos[3] = { hi.P.x, hi.P.y, hi.P.z };
		float nnor[3] = { hi.N.x, hi.N.y, hi.N.z };
		pmap.irradiance_estimate(ir, npos, nnor, .5, 500);
		return Vector3(ir[0], ir[1], ir[2]);
	}
	return Vector3(0);
}

Vector3 Lambert::photonShade(const Ray& ray, const HitInfo& hit,
    const Scene& scene, const Photon_map& pmap) const{
    Vector3 L = Vector3(0.0f, 0.0f, 0.0f);
    const Vector3 viewDir = -ray.d; // d is a unit vector
	float *ir = new float[3];
	float pos[3] = { hit.P.x, hit.P.y, hit.P.z };
	float norm[3] = { hit.N.x, hit.N.y, hit.N.z };
	pmap.irradiance_estimate(ir, pos, norm, .5, 2000);
	Vector3 ira = Vector3(ir[0], ir[1], ir[2]);
	L += m_kd * ira / PI;
    L += m_ka * ra;
	if (trans)
		L += fresnell(ray, hit, scene, pmap);
	if (refra)
		L += calcPhotonRefraction(ray, hit, scene, pmap);
	if (refle)
		L += calcPhotonReflection(ray, hit, scene, pmap);
    //printf("%f\n ", reflection);
    //L += calcPhotonMonteCarlo(ray, hit, scene, pmap);
    return L;
}

Vector3 Lambert::fresnell(const Ray &ray, const HitInfo &hit,
	Scene scene, const Photon_map& pmap) const {
	Ray r;
	HitInfo hi;
	Vector3 reflection;
	Vector3 refraction;
	//Reflection
	if (ray.times < 2) {
		r.o = hit.P;
		r.d = -2 * dot(ray.d, hit.N)*hit.N + ray.d;
		r.o = r.o + r.d * epsilon;
		r.d.normalize();
		r.update();
		r.times = ray.times + 1;
		if (scene.trace(hi, r)) {
			if (hi.t > epsilon)
				reflection += hi.material->photonShade(r, hi, scene, pmap) * m_ks;
		}
	}
	//Refraction
	const Vector3 viewDir = -ray.d; // d is a unit vector
	float dot_ = dot(viewDir, hit.N);
	if (ray.times < 9 && false) {
		r.o = hit.P;
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
					refraction += rf * hi.material->photonShade(r, hi, scene, pmap);
			}
		}
	}
	float _dot = dot( hit.N, r.d);
	float t = (ray.snell * dot_ - this->snell * _dot);
	float rasd = (ray.snell * dot_ + this->snell * _dot);
	float rper = pow((ray.snell * dot_ - this->snell * _dot) / (ray.snell * dot_ + this->snell * _dot),2);
	float rpar = pow((this->snell * dot_ - ray.snell * _dot) / (this->snell * dot_ + ray.snell * _dot), 2);
	float refleConst = (rper + rpar) / 2;
	float refraConst = 1 - refleConst;
	return reflection * refleConst + refraction * refraConst;
}

Vector3 Lambert::calcRefraction(const Ray &ray, const HitInfo &hit, Scene scene) const{
	Vector3 L;
	Ray r;
	HitInfo hi = HitInfo();
	const Vector3 viewDir = -ray.d; // d is a unit vector
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
					L += rf * hi.material->shade(r, hi, scene);
			}
		}
	}
	return L;
}

Vector3 Lambert::calcPhotonRefraction(const Ray &ray, const HitInfo &hit,
	Scene scene, const Photon_map& pmap) const{
	Vector3 L;
	Ray r;
	HitInfo hi = HitInfo();
	const Vector3 viewDir = -ray.d; // d is a unit vector
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
					L += rf * hi.material->photonShade(r, hi, scene, pmap);
			}
		}
	}
	return L;
}

Vector3 Lambert::calcPhotonReflection(const Ray &ray, const HitInfo &hit, 
	Scene scene, const Photon_map& pmap) const{
	Vector3 L;
	Ray r;
	HitInfo hi;
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
				L += hi.material->photonShade(r, hi, scene, pmap) * m_ks;
		}
	}
	return L;
}

Vector3 Lambert::calcReflection(const Ray &ray, const HitInfo &hit, Scene scene) const{
	Vector3 L;
	Ray r;
	HitInfo hi;
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
				L += rs * hi.material->shade(r, hi, scene) * m_ks;
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