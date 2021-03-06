#ifndef CSE168_SCENE_H_INCLUDED
#define CSE168_SCENE_H_INCLUDED

#include "Miro.h"
#include "Object.h"
#include "PointLight.h"
#include "BVH.h"
#include "photonMap.h"

class Camera;
class Image;

class Scene
{
public:
    void addObject(Object* pObj)        {m_objects.push_back(pObj);}
    const Objects* objects() const      {return &m_objects;}

    void addLight(PointLight* pObj)     {m_lights.push_back(pObj);}
    const Lights* lights() const        {return &m_lights;}

    void preCalc();
    void openGL(Camera *cam);
	void buildPhotonMap();
	void normalTrace(Camera *cam, Image *img);
    void raytraceImage(Camera *cam, Image *img);
    bool trace(HitInfo& minHit, const Ray& ray, 
		float tMin = 0.0f, float tMax = MIRO_TMAX) ;
	void setSampleRate(int i) { numOfSamples = i; }
	void toggleDraw() { draw = !draw; }
	void togglePDraw() { pmapd = !pmapd; }
	Ray getReflectedRay(HitInfo minHit, Ray ray);
	Ray getRefractedRay(HitInfo minHit, Ray ray);
	Ray getDiffusedRay(HitInfo minHit, Ray ray);
    void photonTrace(Camera *cam, Image *img);

protected:
	Photon_map *pmap;
	Photon_map *cmap;
	bool pmapd;
	bool draw;
	int numOfSamples;
    Objects m_objects;
    BVH m_bvh;
    Lights m_lights;
};

extern Scene * g_scene;

#endif // CSE168_SCENE_H_INCLUDED
