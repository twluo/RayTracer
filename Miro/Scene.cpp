#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
#include <ctime>
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

    m_bvh.build(&m_objects);
	draw = false;
	int leafCount = 0;
	int nodeCount = 0;
	m_bvh.count(leafCount, nodeCount);
	std::cout << "number of leaf is " << leafCount << " and number of nodes is " << nodeCount << std::endl;
}

void
Scene::raytraceImage(Camera *cam, Image *img)
{
    
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
		printf("Rendering Progress: %.3f%% Time Elapsed: (%f seconds).\r", (++progress) / float(img->height())*100.0f,((float)temp) / CLOCKS_PER_SEC);
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

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) 
{
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}