#include "box.h"


box::box()
{
}

box::box(Vector3 min, Vector3 max) {
	this->min = min;
	this->max = max;
	//print();
}

void box::print() {
	std::cout << "bbmin = " << min << " bbmax = " << max << std::endl;
}
box::~box()
{
}

void drawWireBox(float xmin, float ymin, float zmin, float xmax, float ymax, float zmax) {
	glPushMatrix();
	glTranslatef(0.5f*(xmin + xmax), 0.5f*(ymin + ymax), 0.5f*(zmin + zmax));
	glScalef(xmax - xmin, ymax - ymin, zmax - zmin);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	glutWireCube(1.0f);
	glPopMatrix();
}
void box::draw() {
	drawWireBox(min.x, min.y, min.z, max.x, max.y, max.z);
}
bool box::intersect(HitInfo& minHit, const Ray& ray, float Tmin, float Tmax) const {
	float minX = 0, maxX = 0, minY = 0, maxY = 0, minZ = 0, maxZ = 0, max = 0, min = 0;
	float invrayx = ray.invd.x;
	float invrayy = ray.invd.y;
	float invrayz = ray.invd.z;
	
	if (ray.d.x != 0) {
		float t1 = (this->min.x - ray.o.x) * invrayx;
		float t2 = (this->max.x - ray.o.x) * invrayx;
		minX = fmin(t1, t2);
		maxX = fmax(t1, t2);
	}
	if (ray.d.y != 0) {
		float t1 = (this->min.y - ray.o.y) * invrayy;
		float t2 = (this->max.y - ray.o.y) * invrayy;
		minY = fmin(t1, t2);
		maxY = fmax(t1, t2);
	}
	if (ray.d.z != 0) {
		float t1 = (this->min.z - ray.o.z) * invrayz;
		float t2 = (this->max.z - ray.o.z) * invrayz;
		minZ = fmin(t1, t2);
		maxZ = fmax(t1, t2);
	}
	min = fmax(minZ, fmax(minX, minY));
	max = fmin(maxZ, fmin(maxX, maxY));
	minHit.t = min;
	return max >= fmax(min, 0);
}

//http://people.csail.mit.edu/amy/papers/box-jgt.pdf
//	float tmin, tmax, tymin, tymax, tzmin, tzmax;
//	float invrayx = 1 / ray.d.x;
//	float invrayy = 1 / ray.d.y;
//	float invrayz = 1 / ray.d.z;
//	if (invrayx >= 0) {
//		tmin = (this->min.x - ray.o.x) * invrayx;
//		tmax = (this->max.x - ray.o.x) * invrayx;
//	}
//	else {
//		tmax = (this->min.x - ray.o.x) * invrayx;
//		tmin = (this->max.x - ray.o.x) * invrayx;
//	}
//	if (invrayy >= 0) {
//		tymin = (this->min.y - ray.o.y) * invrayy;
//		tymax = (this->max.y - ray.o.y) * invrayy;
//	}
//	else {
//		tymax = (this->min.y - ray.o.y) * invrayy;
//		tymin = (this->max.y - ray.o.y) * invrayy;
//	}
//	if ((tmin > tymax) || (tymin > tmax))
//		return false;
//	if (tymin > tmin)
//		tmin = tymin;
//	if (tymax < tmax)
//		tmax = tymax;
//	if (invrayz >= 0) {
//		tzmin = (this->min.z - ray.o.z) * invrayz;
//		tzmax = (this->max.z - ray.o.z) * invrayz;
//	}
//	else {
//		tzmax = (this->min.z - ray.o.z) * invrayz;
//		tzmin = (this->max.z - ray.o.z) * invrayz;
//	}
//	if ((tmin > tzmax) || (tzmin > tmax))
//		return false;
//	if (tzmin > tmin)
//		tmin = tzmin;
//	if (tzmax < tmax)
//		tmax = tzmax;
//	return ((tmin < tmax) && (tmax > tmin));