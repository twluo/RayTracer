#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include <ctime>
#include <algorithm>

#define PNUM 1000000
//10000000
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
	if (pmapd){
		pmap->draw();
	}
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
	pmap = new Photon_map(PNUM * m_lights.size());
    m_bvh.build(&m_objects);
	draw = false;
	int leafCount = 0;
	int nodeCount = 0;
	m_bvh.count(leafCount, nodeCount); 
	std::cout << "number of leaf is " << leafCount << " and number of nodes is " << nodeCount << std::endl;
	buildPhotonMap();
	
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
Scene::buildPhotonMap() {
	const Lights *lightlist = lights();
	Lights::const_iterator lightIter; 
	Ray r;
	int pnum = 0;
	clock_t t;
	t = clock();

	for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++) {
		int stored = 0;
		while (stored <= PNUM) {
			PointLight* pLight = *lightIter;
			r.o = pLight->position();
			pnum++;
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
			r.power = pLight->wattage() * pLight->color();
			HitInfo hit;
			while (true) {
				if (trace(hit, r)) {
					float pos[3] = { hit.P.x, hit.P.y, hit.P.z };
					float dir[3] = { r.d.x, r.d.y, r.d.z };
					float pow[3] = { r.power.x, r.power.y, r.power.z };
					pmap->store(pow, pos, dir);
					stored++;
					Vector3 k_d = hit.material->m_kd;
					Vector3 k_s = hit.material->m_ks;

					float rs = std::max(k_d.x * k_s.x, std::max(k_d.y * k_s.y, k_d.z * k_s.z));
					float rd = (k_d.x + k_d.y + k_d.z) / (k_d.x + k_d.y + k_d.z + k_s.x + k_s.y + k_s.z);
					float ran = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
					if (ran < rd) {
						//printf("DIFFUSED\n");
						Ray nr = getDiffusedRay(hit, r);
						r = nr;
						r.power = Vector3(pow[0], pow[1], pow[2]) * hit.material->m_kd;
					}
					else if (ran < rd + rs) {
						//printf("REFLECTED\n");
						Ray nr = getReflectedRay(hit, r);
						r = nr;
						r.power = Vector3(pow[0], pow[1], pow[2]) * hit.material->m_ks;
					}
					else {
						//printf("ABSORBED\n");
						break;
					}
				}
				else
					break;
			}
		}
	}
	clock_t temp = clock() - t;
	printf("Done emitting photons, Time Elapsed: (%f seconds).\n", ((float)temp) / CLOCKS_PER_SEC);

	t = clock();
	float scale = pnum;
	scale = 1 / scale;
	pmap->scale_photon_power(scale);
	pmap->balance();
	temp = clock() - t;
	printf("Done Balancing, Time Elapsed: (%f seconds).\n", ((float)temp) / CLOCKS_PER_SEC);
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
Scene::photonTrace(Camera *cam, Image *img){
    bool shadow = true;
    const Lights *lightlist = this->lights();
    clock_t t;
    t = clock();
    int rayCount = 0;
    int progress = 0;
    int bounceCount = 0;
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
	{
		#pragma omp parallel for
        for (int i = 0; i < img->width(); ++i)
        {
            Vector3 shadeResult;
            Ray ray;
            HitInfo hitInfo;
            Ray altRay;
            HitInfo altHitInfo;
            shadeResult = Vector3(0);
            for (int k = 0; k < numOfSamples; k++) {
				//if (k == 0)
                    //ray = cam->eyeRay(i, j, img->width(), img->height());
                //else
                    //ray = cam->randomRay(i, j, img->width(), img->height());
				ray = cam->randomFOVRay(i, j, img->width(), img->height());
                rayCount++;
                if (trace(hitInfo, ray))
                {
					shadeResult += hitInfo.material->photonShade(ray, hitInfo, *this, *pmap);
					/*float *ir = new float[3];
					float pos[3] = { hitInfo.P.x, hitInfo.P.y, hitInfo.P.z };
					float norm[3] = { hitInfo.N.x, hitInfo.N.y, hitInfo.N.z };
					pmap->irradiance_estimate(ir, pos, norm, 0.1, 500);
					shadeResult *= Vector3(ir[0], ir[1], ir[2]);*/
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
	
	//normalTrace(cam, img);
    photonTrace(cam, img);
    
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) 
{
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}