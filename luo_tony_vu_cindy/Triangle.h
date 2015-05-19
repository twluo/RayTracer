#ifndef CSE168_TRIANGLE_H_INCLUDED
#define CSE168_TRIANGLE_H_INCLUDED

#include "TriangleMesh.h"
#include "Object.h"

/*
    The Triangle class stores a pointer to a mesh and an index into its
    triangle array. The mesh stores all data needed by this Triangle.
*/
class Triangle : public Object
{
public:
    Triangle(TriangleMesh * m = 0, unsigned int i = 0);
    virtual ~Triangle();

    void setIndex(unsigned int i) {m_index = i;}
    void setMesh(TriangleMesh* m) {m_mesh = m;}
	void calculateCenteroid();
	virtual void preCalc() {
		calculateCenteroid();
		hit = false;
	}
	Vector3 getCenteroid() { return centeroid; }
	Vector3 getMax() {
		TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
		const Vector3 & A = m_mesh->vertices()[ti3.x]; //vertex a of triangle
		const Vector3 & B = m_mesh->vertices()[ti3.y]; //vertex b of triangle
		const Vector3 & C = m_mesh->vertices()[ti3.z]; //vertex c of triangle 
		Vector3 max;
		max.x = fmax(A.x, fmax(B.x, C.x));
		max.y = fmax(A.y, fmax(B.y, C.y));
		max.z = fmax(A.z, fmax(B.z, C.z));
		return max;
	}
	Vector3 getMin() {
		TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
		const Vector3 & A = m_mesh->vertices()[ti3.x]; //vertex a of triangle
		const Vector3 & B = m_mesh->vertices()[ti3.y]; //vertex b of triangle
		const Vector3 & C = m_mesh->vertices()[ti3.z]; //vertex c of triangle 
		Vector3 min;
		min.x = fmin(A.x, fmin(B.x, C.x));
		min.y = fmin(A.y, fmin(B.y, C.y));
		min.z = fmin(A.z, fmin(B.z, C.z));
		return min;
	}
    virtual void renderGL();
    virtual bool intersect(HitInfo& result, const Ray& ray,
                           float tMin = 0.0f, float tMax = MIRO_TMAX);
    
protected:
    TriangleMesh* m_mesh;
    unsigned int m_index;
	Vector3 centeroid;
};

#endif // CSE168_TRIANGLE_H_INCLUDED
