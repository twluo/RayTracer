#include <math.h>
#include "Miro.h"
#include "Scene.h"
#include "Camera.h"
#include "Image.h"
#include "Console.h"

#include "PointLight.h"
#include "Sphere.h"
#include "TriangleMesh.h"
#include "Triangle.h"
#include "Lambert.h"
#include "MiroWindow.h"

void
makeSpiralScene()
{
    g_camera = new Camera;
    g_scene = new Scene;
    g_image = new Image;

    g_image->resize(512, 512);
    
    // set up the camera
    g_camera->setBGColor(Vector3(1.0f, 1.0f, 1.0f));
    g_camera->setEye(Vector3(-5, 2, 3));
    g_camera->setLookAt(Vector3(0, 0, 0));
    g_camera->setUp(Vector3(0, 1, 0));
    g_camera->setFOV(45);

    // create and place a point light source
    PointLight * light = new PointLight;
    light->setPosition(Vector3(-3, 15, 3));
    light->setColor(Vector3(1, 1, 1));
    light->setWattage(1000);
    g_scene->addLight(light);

    // create a spiral of spheres
    Material* mat = new Lambert(Vector3(1.0f, 0.0f, 0.0f));
    const int maxI = 2000;
    const float a = 0.15f;
    for (int i = 1; i < maxI; ++i)
    {
        float t = i/float(maxI);
        float theta = 4*PI*t;
        float r = a*theta;
        float x = r*cos(theta);
        float y = r*sin(theta);
        float z = 2*(2*PI*a - r);
        Sphere * sphere = new Sphere;
        sphere->setCenter(Vector3(x,y,z));
        sphere->setRadius(r/10);
        sphere->setMaterial(mat);
        g_scene->addObject(sphere);
    }
    
    // let objects do pre-calculations if needed
    g_scene->preCalc();
}



// local helper function declarations
namespace
{
	void addMeshTrianglesToScene(TriangleMesh * mesh, Material * material);
	inline Matrix4x4 translate(float x, float y, float z);
	inline Matrix4x4 scale(float x, float y, float z);
	inline Matrix4x4 rotate(float angle, float x, float y, float z);
} // namespace

void
makeCornellScene()
{
	g_camera = new Camera;
	g_scene = new Scene;
	g_image = new Image;

	g_image->resize(1024, 1024);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
	g_camera->setEye(Vector3(2.75, 2.75, 8));
	g_camera->setLookAt(Vector3(2.75, 2.75, -2.75));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);
	g_camera->setASize(0.25);
	g_camera->setFDist(10.75);

	// create and place a point light source
	PointLight * light = new PointLight;
	light->setPosition(Vector3(2.75, 5.4, -2.75));
	light->setColor(Vector3(1, 1, 1));
	light->setWattage(200);
	g_scene->addLight(light);


	PointLight * light1 = new PointLight;
	light1->setPosition(Vector3(2.75, 2, 1));
	light1->setColor(Vector3(1, 1, 1));
	light1->setWattage(150);
	g_scene->addLight(light1);

	Material* wmat = new Lambert();
	wmat->setDiffuse(Vector3(1, 1, 1), 1);
	
	Material* bmat = new Lambert();
	bmat->setDiffuse(Vector3(0, 0, 1), 1);

	Material* rgmat = new Lambert();
	rgmat->setDiffuse(Vector3(1, 1, 0), 1);

	Material* rmat = new Lambert();
	rmat->setDiffuse(Vector3(1, 0, 0), 1);

	Material* gmat = new Lambert();
	gmat->setDiffuse(Vector3(0, 1, 0), 1);

	Material* glass = new Lambert();
	glass->setDiffuse(Vector3(0, 0, 0), 0);
	glass->setSpecular(Vector3(1, 1, 1), 1);
	glass->setRefra(1, 1.33);

	Material* mirror = new Lambert();
	mirror->setDiffuse(Vector3(0, 0, 0), 0);
	mirror->setSpecular(Vector3(1, 1, 1), 1);
	mirror->setRefle();

	TriangleMesh * bunny = new TriangleMesh;

	bunny->load("cornell_box.obj");

	// create all the triangles in the bunny mesh and add to the scene
	//for (int i = 0; i < bunny->numTris; ++i)
	for (int i = 0; i < 16; ++i)
	{
		Triangle* t = new Triangle;
		t->setIndex(i);
		t->setMesh(bunny);
		if (i >= 4 && i < 6)
			t->setMaterial(rmat);
		else if (i >= 6 && i < 8)
			t->setMaterial(gmat);
		else if (i >= 16 && i < 27)
			t->setMaterial(wmat);
		else if (i >= 27)
			t->setMaterial(wmat);
		else
			t->setMaterial(wmat);
		t->calculateCenteroid();
		g_scene->addObject(t);
	}

	
	/*Sphere* greenSphere = new Sphere;
	greenSphere->setCenter(Vector3(1.75, 2.75, -3.75));
	greenSphere->setRadius(0.75);
	greenSphere->setMaterial(gmat);
	g_scene->addObject(greenSphere);
	Sphere* mirrorSphere = new Sphere;
	mirrorSphere->setCenter(Vector3(2.75, 2.75, 1.75));
	mirrorSphere->setRadius(1);
	mirrorSphere->setMaterial(glass);
	g_scene->addObject(mirrorSphere);
	Sphere* redSphere = new Sphere;
	redSphere->setCenter(Vector3(3.75, 2.75, -3.75));
	redSphere->setRadius(0.75);
	redSphere->setMaterial(rmat);
	g_scene->addObject(redSphere); */
	Sphere* metalSphere = new Sphere;
	metalSphere->setCenter(Vector3(5.5, 5.5, -5.5));
	metalSphere->setRadius(1);
	metalSphere->setMaterial(mirror);
	g_scene->addObject(metalSphere);
	Sphere* metalSphere1 = new Sphere;
	metalSphere1->setCenter(Vector3(0, 5.5, -5.5));
	metalSphere1->setRadius(1);
	metalSphere1->setMaterial(mirror);
	g_scene->addObject(metalSphere1);
	/*Sphere* metalSphere2 = new Sphere;
	metalSphere2->setCenter(Vector3(5.5, 0, -5.5));
	metalSphere2->setRadius(1);
	metalSphere2->setMaterial(mirror);
	g_scene->addObject(metalSphere2);
	Sphere* metalSphere3 = new Sphere;
	metalSphere3->setCenter(Vector3(0, 0, -5.5));
	metalSphere3->setRadius(1);
	metalSphere3->setMaterial(mirror);
	g_scene->addObject(metalSphere3);*/
	// let objects do pre-calculations if needed

	/*Sphere* sphere1 = new Sphere;
	sphere1->setCenter(Vector3(0.35, 0.35, -0.35));
	sphere1->setRadius(0.1);
	sphere1->setMaterial(gmat);
	g_scene->addObject(sphere1);
	Sphere* sphere2 = new Sphere;
	sphere2->setCenter(Vector3(0.65, 0.65, -0.65));
	sphere2->setRadius(0.2);
	sphere2->setMaterial(gmat);
	g_scene->addObject(sphere2);
	Sphere* sphere3 = new Sphere;
	sphere3->setCenter(Vector3(1.15, 1.15, -1.15));
	sphere3->setRadius(0.3);
	sphere3->setMaterial(gmat);
	g_scene->addObject(sphere3);
	Sphere* sphere4 = new Sphere;
	sphere4->setCenter(Vector3(1.85, 1.85, -1.85));
	sphere4->setRadius(0.4);
	sphere4->setMaterial(gmat);
	g_scene->addObject(sphere4);
	Sphere* sphere5 = new Sphere;
	sphere5->setCenter(Vector3(2.75, 2.75, -2.75));
	sphere5->setRadius(0.5);
	sphere5->setMaterial(mirror);
	g_scene->addObject(sphere5);
	Sphere* sphere6 = new Sphere;
	sphere6->setCenter(Vector3(3.65, 3.65, -3.65));
	sphere6->setRadius(0.4);
	sphere6->setMaterial(rmat);
	g_scene->addObject(sphere6);
	Sphere* sphere7 = new Sphere;
	sphere7->setCenter(Vector3(4.35, 4.35, -4.35));
	sphere7->setRadius(0.3);
	sphere7->setMaterial(rmat);
	g_scene->addObject(sphere7);
	Sphere* sphere8 = new Sphere;
	sphere8->setCenter(Vector3(4.85, 4.85, -4.85));
	sphere8->setRadius(0.2);
	sphere8->setMaterial(rmat);
	g_scene->addObject(sphere8);
	Sphere* sphere9 = new Sphere;
	sphere9->setCenter(Vector3(5.15, 5.15, -5.15));
	sphere9->setRadius(0.1);
	sphere9->setMaterial(rmat);
	g_scene->addObject(sphere9);*/

	
	const int maxI = 2000;
	const float a = 0.15f;
	for (int i = 1; i < maxI; ++i)
	{
		float t = i / float(maxI);
		float theta = 4 * PI*t;
		float r = a*theta;
		float x = r*cos(theta);
		float y = r*sin(theta);
		x += 2.75;
		y += 3.5;
		float z = -3.25;
		Sphere * sphere = new Sphere;
		sphere->setCenter(Vector3(x, y, z));
		sphere->setRadius(r / 10);
		sphere->setMaterial(mirror);
		g_scene->addObject(sphere);
	}
	Sphere* glassSphere = new Sphere;
	glassSphere->setCenter(Vector3(1, 1, -1.75));
	glassSphere->setRadius(0.75);
	glassSphere->setMaterial(glass);
	g_scene->addObject(glassSphere);
	Sphere* mirrorSphere = new Sphere;
	mirrorSphere->setCenter(Vector3(2.75, 1, -1.75));
	mirrorSphere->setRadius(0.75);
	mirrorSphere->setMaterial(mirror);
	g_scene->addObject(mirrorSphere);
	Sphere* diffuseSphere = new Sphere;
	diffuseSphere->setCenter(Vector3(4.50, 1, -1.75));
	diffuseSphere->setRadius(0.75);
	diffuseSphere->setMaterial(bmat);
	g_scene->addObject(diffuseSphere);

	g_scene->preCalc();
	g_scene->setSampleRate(100);
}
void
makeTeapotScene()
{
	g_camera = new Camera;
	g_scene = new Scene;
	g_image = new Image;

	g_image->resize(512, 512);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 1.0f, 0.0f));
	g_camera->setEye(Vector3(-2, 3, 5));
	g_camera->setLookAt(Vector3(-.5, 1, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);
	/*
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
	g_camera->setEye(Vector3(0, 3, 6));
	g_camera->setLookAt(Vector3(0, 0, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);*/

	// create and place a point light source
	PointLight * light = new PointLight;
	//light->setPosition(Vector3(10, 10, 10));
	light->setPosition(Vector3(-3, 15, 3));
	light->setColor(Vector3(1, 1, 1));
	light->setWattage(500);
	g_scene->addLight(light);

	Material* material = new Lambert(Vector3(1.0f));
	material->setDiffuse(Vector3(0.0f,0.0f,0.0f), 0);
	material->setSpecular(Vector3(1.0f,1.0f,1.0f), 1);
	material->setAmbient(Vector3(0.0f), 0.05);
	material->setRefra(1, 1.33);
	/*Matrix4x4 xmat;
	xmat *= scale(0.5, 0.5, 0.5); 
	TriangleMesh * teapot = new TriangleMesh;
	teapot->load("bear.obj", xmat);
	addMeshTrianglesToScene(teapot, material);*/

	Sphere* mirrorSphere = new Sphere;
	mirrorSphere->setCenter(Vector3(2.75, 2, -2.75));
	mirrorSphere->setRadius(0.75);
	mirrorSphere->setMaterial(material);
	g_scene->addObject(mirrorSphere);

	// create the floor triangle
	Material* material1 = new Lambert(Vector3(1.0f,0.0f,0.0f));
	material1->setDiffuse(Vector3(1.0f,0.0f,0.0f), 0.5);
	material1->setSpecular(Vector3(1.0f), 1);
	material1->setAmbient(Vector3(1.0f), 0.05);
	TriangleMesh * floor = new TriangleMesh;
	floor->createSingleTriangle();
	floor->setV1(Vector3(-10, 0, -10));
	floor->setV2(Vector3(0, 0, 10));
	floor->setV3(Vector3(10, 0, -10));
	floor->setN1(Vector3(0, 1, 0));
	floor->setN2(Vector3(0, 1, 0));
	floor->setN3(Vector3(0, 1, 0));

	Triangle* t = new Triangle;
	t->setIndex(0);
	t->setMesh(floor);
	t->setMaterial(material1);
	g_scene->addObject(t);

	// let objects do pre-calculations if needed
	g_scene->preCalc();
	g_scene->setSampleRate(1);
}


void
makeBunny1Scene()
{
	g_camera = new Camera;
	g_scene = new Scene;
	g_image = new Image;

	g_image->resize(512, 512);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
	g_camera->setEye(Vector3(0, 5, 15));
	g_camera->setLookAt(Vector3(0, 0, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);

	// create and place a point light source
	PointLight * light = new PointLight;
	light->setPosition(Vector3(10, 20, 10));
	light->setColor(Vector3(1, 1, 1));
	light->setWattage(1000);
	g_scene->addLight(light);

	Material* material = new Lambert(Vector3(1.0f));
	TriangleMesh * bunny = new TriangleMesh;
	bunny->load("bunny.obj");
	addMeshTrianglesToScene(bunny, material);

	// create the floor triangle
	TriangleMesh * floor = new TriangleMesh;
	floor->createSingleTriangle();
	floor->setV1(Vector3(-100, 0, -100));
	floor->setV2(Vector3(0, 0, 100));
	floor->setV3(Vector3(100, 0, -100));
	floor->setN1(Vector3(0, 1, 0));
	floor->setN2(Vector3(0, 1, 0));
	floor->setN3(Vector3(0, 1, 0));

	Triangle* t = new Triangle;
	t->setIndex(0);
	t->setMesh(floor);
	t->setMaterial(material);
	g_scene->addObject(t);

	// let objects do pre-calculations if needed
	g_scene->preCalc();
	g_scene->setSampleRate(1);
}



void
makeBunny20Scene()
{
	g_camera = new Camera;
	g_scene = new Scene;
	g_image = new Image;

	g_image->resize(512, 512);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
	g_camera->setEye(Vector3(0, 5, 15));
	g_camera->setLookAt(Vector3(0, 0, 0));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(45);

	// create and place a point light source
	PointLight * light = new PointLight;
	light->setPosition(Vector3(10, 20, 10));
	light->setColor(Vector3(1, 1, 1));
	light->setWattage(1000);
	g_scene->addLight(light);

	TriangleMesh * mesh;
	Material* material = new Lambert(Vector3(1.0f));
	Matrix4x4 xform;
	Matrix4x4 xform2;
	xform2 *= rotate(110, 0, 1, 0);
	xform2 *= scale(.6, 1, 1.1);


	// bunny 1
	xform.setIdentity();
	xform *= scale(0.3, 2.0, 0.7);
	xform *= translate(-1, .4, .3);
	xform *= rotate(25, .3, .1, .6);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 2
	xform.setIdentity();
	xform *= scale(.6, 1.2, .9);
	xform *= translate(7.6, .8, .6);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 3
	xform.setIdentity();
	xform *= translate(.7, 0, -2);
	xform *= rotate(120, 0, .6, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 4
	xform.setIdentity();
	xform *= translate(3.6, 3, -1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 5
	xform.setIdentity();
	xform *= translate(-2.4, 2, 3);
	xform *= scale(1, .8, 2);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 6
	xform.setIdentity();
	xform *= translate(5.5, -.5, 1);
	xform *= scale(1, 2, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 7
	xform.setIdentity();
	xform *= rotate(15, 0, 0, 1);
	xform *= translate(-4, -.5, -6);
	xform *= scale(1, 2, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 8
	xform.setIdentity();
	xform *= rotate(60, 0, 1, 0);
	xform *= translate(5, .1, 3);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 9
	xform.setIdentity();
	xform *= translate(-3, .4, 6);
	xform *= rotate(-30, 0, 1, 0);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 10
	xform.setIdentity();
	xform *= translate(3, 0.5, -2);
	xform *= rotate(180, 0, 1, 0);
	xform *= scale(1.5, 1.5, 1.5);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 11
	xform = xform2;
	xform *= scale(0.3, 2.0, 0.7);
	xform *= translate(-1, .4, .3);
	xform *= rotate(25, .3, .1, .6);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 12
	xform = xform2;
	xform *= scale(.6, 1.2, .9);
	xform *= translate(7.6, .8, .6);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 13
	xform = xform2;
	xform *= translate(.7, 0, -2);
	xform *= rotate(120, 0, .6, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 14
	xform = xform2;
	xform *= translate(3.6, 3, -1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 15
	xform = xform2;
	xform *= translate(-2.4, 2, 3);
	xform *= scale(1, .8, 2);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 16
	xform = xform2;
	xform *= translate(5.5, -.5, 1);
	xform *= scale(1, 2, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 17
	xform = xform2;
	xform *= rotate(15, 0, 0, 1);
	xform *= translate(-4, -.5, -6);
	xform *= scale(1, 2, 1);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 18
	xform = xform2;
	xform *= rotate(60, 0, 1, 0);
	xform *= translate(5, .1, 3);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 19
	xform = xform2;
	xform *= translate(-3, .4, 6);
	xform *= rotate(-30, 0, 1, 0);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);

	// bunny 20
	xform = xform2;
	xform *= translate(3, 0.5, -2);
	xform *= rotate(180, 0, 1, 0);
	xform *= scale(1.5, 1.5, 1.5);
	mesh = new TriangleMesh;
	mesh->load("bunny.obj", xform);
	addMeshTrianglesToScene(mesh, material);


	// create the floor triangle
	mesh = new TriangleMesh;
	mesh->createSingleTriangle();
	mesh->setV1(Vector3(-100, 0, -100));
	mesh->setV2(Vector3(0, 0, 100));
	mesh->setV3(Vector3(100, 0, -100));
	mesh->setN1(Vector3(0, 1, 0));
	mesh->setN2(Vector3(0, 1, 0));
	mesh->setN3(Vector3(0, 1, 0));

	Triangle* t = new Triangle;
	t->setIndex(0);
	t->setMesh(mesh);
	t->setMaterial(material);
	g_scene->addObject(t);

	// let objects do pre-calculations if needed
	g_scene->preCalc();
}


void
makeSponzaScene()
{
	g_camera = new Camera;
	g_scene = new Scene;
	g_image = new Image;

	g_image->resize(512, 512);

	// set up the camera
	g_camera->setBGColor(Vector3(0.0f, 0.0f, 0.2f));
	g_camera->setEye(Vector3(8, 1.5, 1));
	g_camera->setLookAt(Vector3(0, 2.5, -1));
	g_camera->setUp(Vector3(0, 1, 0));
	g_camera->setFOV(55);

	// create and place a point light source
	PointLight * light = new PointLight;
	light->setPosition(Vector3(0, 10.0, 0));
	light->setColor(Vector3(1, 1, 1));
	light->setWattage(200);
	g_scene->addLight(light);

	Material* material = new Lambert(Vector3(1.0f));
	TriangleMesh * mesh = new TriangleMesh;
	mesh->load("sponza.obj");
	addMeshTrianglesToScene(mesh, material);

	// let objects do pre-calculations if needed
	g_scene->preCalc();
	g_scene->setSampleRate(1);
}

// local helper function definitions
namespace
{

	void
		addMeshTrianglesToScene(TriangleMesh * mesh, Material * material)
	{
		// create all the triangles in the bunny mesh and add to the scene
		for (int i = 0; i < mesh->numTris(); ++i)
		{
			Triangle* t = new Triangle;
			t->setIndex(i);
			t->setMesh(mesh);
			t->setMaterial(material);
			g_scene->addObject(t);
		}
	}


	inline Matrix4x4
		translate(float x, float y, float z)
	{
		Matrix4x4 m;
		m.setColumn4(Vector4(x, y, z, 1));
		return m;
	}


	inline Matrix4x4
		scale(float x, float y, float z)
	{
		Matrix4x4 m;
		m.m11 = x;
		m.m22 = y;
		m.m33 = z;
		return m;
	}

	// angle is in degrees
	inline Matrix4x4
		rotate(float angle, float x, float y, float z)
	{
		float rad = angle*(PI / 180.);

		float x2 = x*x;
		float y2 = y*y;
		float z2 = z*z;
		float c = cos(rad);
		float cinv = 1 - c;
		float s = sin(rad);
		float xy = x*y;
		float xz = x*z;
		float yz = y*z;
		float xs = x*s;
		float ys = y*s;
		float zs = z*s;
		float xzcinv = xz*cinv;
		float xycinv = xy*cinv;
		float yzcinv = yz*cinv;

		Matrix4x4 m;
		m.set(x2 + c*(1 - x2), xy*cinv + zs, xzcinv - ys, 0,
			xycinv - zs, y2 + c*(1 - y2), yzcinv + xs, 0,
			xzcinv + ys, yzcinv - xs, z2 + c*(1 - z2), 0,
			0, 0, 0, 1);
		return m;
	}

} // namespace

int
main(int argc, char*argv[])
{
    // create a scene
    makeCornellScene();

    MiroWindow miro(&argc, argv);
    miro.mainLoop();

    return 0; // never executed
}

