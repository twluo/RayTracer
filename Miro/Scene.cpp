#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"
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

    glutSwapBuffers();
}

void
Scene::preCalc()
{
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
}

void
Scene::raytraceImage(Camera *cam, Image *img)
{
    Ray ray;
    HitInfo hitInfo;
    Ray altRay;
    HitInfo altHitInfo;
    bool shadow = false;
    Vector3 shadeResult;
    const Lights *lightlist = this->lights();
    // loop over all pixels in the image
    for (int j = 0; j < img->height(); ++j)
    {
        for (int i = 0; i < img->width(); ++i)
        {
            ray = cam->eyeRay(i, j, img->width(), img->height());
            if (trace(hitInfo, ray))
            {
                shadeResult = hitInfo.material->shade(ray, hitInfo, *this);
                if (shadow) {
                    Lights::const_iterator lightIter;
                    ray.o = hitInfo.P;
                    for (lightIter = lightlist->begin(); lightIter != lightlist->end(); lightIter++)
                    {
                        PointLight* pLight = *lightIter;
                        ray.d = pLight->position() - hitInfo.P;
                        if (trace(hitInfo, ray)) {
                            if (hitInfo.t > EPSILON) {
                                shadeResult = Vector3(0);
                                break;
                            }
                        }
                    }
                }
                img->setPixel(i, j, shadeResult);
                Lights::const_iterator lightIter;
                /*for (lightIter = m_lights.begin(); lightIter != m_lights.end(); lightIter++)
                {
                PointLight* pLight = *lightIter;
                Vector3 l = pLight->position() - hitInfo.P;
                Ray r = Ray(hitInfo.P, l);
                if (trace(hitInfo, r)){
                img->setPixel(i, j, Vector3(0));
                }
                }*/

            }
        }
        img->drawScanline(j);
        glFinish();
        printf("Rendering Progress: %.3f%%\r", j / float(img->height())*100.0f);
        fflush(stdout);
    }

    printf("Rendering Progress: 100.000%\n");
    debug("done Raytracing!\n");
}

bool
Scene::trace(HitInfo& minHit, const Ray& ray, float tMin, float tMax) const
{
    return m_bvh.intersect(minHit, ray, tMin, tMax);
}