#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include <ctime>

#define PNUM 200
#define IPNUM 1/PNUM
#define EPSILON 0.0001

Scene * g_scene = 0;

void
Scene::openGL(Camera *cam)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    cam->drawGL();

    // draw objects
    for (size_t i = 0; i < m_objects.size(); ++i)
        m_objects[i]->renderGL();
	if (draw)
		m_bvh.draw();

    glutSwapBuffers();
}

void
Scene::preCalc()
{
	numOfSamples = 1;
    Objects::iterator it;
    for (it = m_objects.begin(); it != m_objects.end(); it++)
    {
        Object* pObject = *it;
        pObject->preCalc();
    }
    Lights::iterator lit;
    for (lit = m_lights.begin(); lit != m_lights.end(); lit++)
    {
        PointLight* pLight = *lit;
        pLight->preCalc();
    }
	pmap = new Photon_map(PNUM * 5);
    m_bvh.build(&m_objects);
	draw = false;
	int leafCount = 0;
	int nodeCount = 0;
	m_bvh.count(leafCount, nodeCount);
	std::cout << "number of leaf is " << leafCount << " and number of nodes is " << nodeCount << std::endl;
}

Ray Scene::getReflectedRay(HitInfo hit, Ray ray) {
	Ray r;
	r.o = hit.P;
	r.d = -2 * dot(ray.d, hit.N)*hit.N + ray.d;
	r.o = r.o + r.d * epsilon;
	r.d.normalize();
	r.update();
	return r;
}

Ray Scene::getRefractedRay(HitInfo hit, Ray ray) {
	Ray r;
	const Vector3 viewDir = -ray.d; // d is a unit vector
	float dot_ = dot(viewDir, hit.N);
	float ratio;
	ratio = ray.snell / hit.material->snell;
	if (dot_ > 0.0f) {
		r.snell = hit.material->snell;
	}
	else {
		r.snell = 1.0;
		dot_ = -dot_;
	}
	float temp = (1 - pow(ratio, 2) * (1 - pow(dot_, 2)));
	if (temp >= 0) {
		r.o = hit.P;
		r.d = -1 * ratio * (viewDir - dot_ * hit.N) - sqrt(temp)  * hit.N;
		r.o = r.o + r.d * epsilon;
		r.d.normalize();
		r.update();
	}
	return r;
}
Vector3 hemisphereSample(float u, float v) {
	float phi = v * 2.0 * PI;
	float cosTheta = sqrt(1.0 - u);
	float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
	return Vector3(cos(phi) * sinTheta, sin(phi) * sinTheta, cosTheta);
}
Ray Scene::getDiffusedRay(HitInfo hit, Ray ray) {
	Ray r;
	float v = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float w = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	Vector3 dir = hemisphereSample(v, w);

	float rx = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float ry = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	float rz = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
	Vector3 rand = Vector3(rx, ry, rz);
	rand.normalize();
	Vector3 n = hit.N;
	Vector3 t = cross(rand, n);
	Vector3 s = cross(n, t);
	r.o = hit.P;
	r.d = dir.x * t + dir.z * n + dir.y * s;
	r.d.normalize();
	r.update();
	return r;
}
void
Scene::photonTrace(Camera *cam, Image *img) {
	const Lights *lightlist = lights();
	Lights::const_iterator lightIter; 
	Ray r;
	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++) {
		for (int i = 0; i < 2000; i++) {
			PointLight* pLight = *lightIter;
			r.o = pLight->position();
			float x;
			float y;
			float z;
			do {
				x = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2 - 1;
				y = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2 - 1;
				z = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2 - 1;
			} while ((x*x + y*y + z*z) > 1);
			r.d = Vector3(x, y, z);
			r.d.normalize();
			r.update();
			r.power = pLight->wattage() * IPNUM;
			HitInfo hit;
			while (true) {
				if (trace(hit, r)) {
					float pos[3] = { hit.P.x, hit.P.y, hit.P.z };
					float dir[3] = { r.d.x, r.d.y, r.d.z };
					float pow[3] = { r.power, r.power, r.power };
					pmap->store(pow, pos, dir);
					float rd = hit.material->rd;
					float rs = hit.material->rs;
					float rf = hit.material->rf;
					float ran = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
					if (ran < rs) {
						float nr = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * (rd + rs);
						if (nr < rd) {
							Ray nr = getDiffusedRay(hit, r);
							r = nr;
						}
						else {
							Ray nr = getReflectedRay(hit, r);
							r = nr;
						}
					}
					else 
						break;
				}
				else
					break;
			}
		}
	}
}

void
Scene::normalTrace(Camera *cam, Image *img) {
	bool shadow = true;
	const Lights *lightlist = this->lights();
	clock_t t;
	t = clock();
	int rayCount = 0;
	int progress = 0;
	int bounceCount = 0;
	// loop over all pixels in the image
#pragma omp parallel for
	for (int j = 0; j < img->height(); ++j)
	{
		for (int i = 0; i < img->width(); ++i)
		{
			Vector3 shadeResult;
			Ray ray;
			HitInfo hitInfo;
			Ray altRay;
			HitInfo altHitInfo;
			shadeResult = Vector3(0);
			for (int k = 0; k < numOfSamples; k++) {
				if (k == 0)
					ray = cam->eyeRay(i, j, img->width(), img->height());
				else
					ray = cam->randomRay(i, j, img->width(), img->height());
				rayCount++;
				if (trace(hitInfo, ray))
				{
					shadeResult += hitInfo.material->shade(ray, hitInfo, *this);
					if (!shadow) {
						Lights::const_iterator lightIter;
						ray.o = hitInfo.P;
						for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
						{
							PointLight* pLight = *lightIter;
							ray.d = pLight->position() - hitInfo.P;
							ray.d.normalize();
							ray.update();
							ray.o = ray.o + ray.d * epsilon;
							if (trace(hitInfo, ray)) {
								if (hitInfo.t > EPSILON) {
									shadeResult = Vector3(0);
									break;
								}
							}
						}
					}
				}
				else
					shadeResult += Vector3(0);
			}
			shadeResult /= numOfSamples;
			img->setPixel(i, j, shadeResult);
		}
		img->drawScanline(j);
		glFinish();
		clock_t temp = clock() - t;
		printf("Rendering Progress: %.3f%% Time Elapsed: (%f seconds).\r", (++progress) / float(img->height())*100.0f, ((float)temp) / CLOCKS_PER_SEC);
		fflush(stdout);
	}
	t = clock() - t;
	printf("Rendering Progress: 100.000%% Time Elapsed: (%f seconds).\n", ((float)t) / CLOCKS_PER_SEC);
	int RL = 0;
	int RB = 0;
	m_bvh.countIntersections(RB, RL);
	printf("There were %d rays\n", rayCount);
	printf("There were %d pixels\n", img->height() * img->width());
	printf("There were %d Ray-Box Intersections and %d Ray-Triangle Intersections \n", RB, RL);
	debug("done Raytracing!\n");
}
void
Scene::raytraceImage(Camera *cam, Image *img)
{
	photonTrace(cam, img);
	printf("%f\n",pmap->photons[0].pos[0]);
	normalTrace(cam, img);
    
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) 
{
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}