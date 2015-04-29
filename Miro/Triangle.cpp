#include "Triangle.h"
#include "TriangleMesh.h"
#include "Ray.h"
#define EPSILON 0.000001


Triangle::Triangle(TriangleMesh * m, unsigned int i) :
    m_mesh(m), m_index(i)
{

}


Triangle::~Triangle()
{

}


void
Triangle::renderGL()
{
    TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
    const Vector3 & v0 = m_mesh->vertices()[ti3.x]; //vertex a of triangle
    const Vector3 & v1 = m_mesh->vertices()[ti3.y]; //vertex b of triangle
    const Vector3 & v2 = m_mesh->vertices()[ti3.z]; //vertex c of triangle

    glBegin(GL_TRIANGLES);
        glVertex3f(v0.x, v0.y, v0.z);
        glVertex3f(v1.x, v1.y, v1.z);
        glVertex3f(v2.x, v2.y, v2.z);
    glEnd();
}



bool
Triangle::intersect(HitInfo& result, const Ray& r,float tMin, float tMax)
{
    //All the vertices of the triangle
    TriangleMesh::TupleI3 ti3 = m_mesh->vIndices()[m_index];
    const Vector3 & A = m_mesh->vertices()[ti3.x]; //vertex a of triangle
    const Vector3 & B = m_mesh->vertices()[ti3.y]; //vertex b of triangle
    const Vector3 & C = m_mesh->vertices()[ti3.z]; //vertex c of triangle
    TriangleMesh::TupleI3 ti4 = m_mesh->nIndices()[m_index];
    const Vector3 & n_A = m_mesh->normals()[ti4.x]; //normal a of triangle
    const Vector3 & n_B = m_mesh->normals()[ti4.y]; //normal b of triangle
    const Vector3 & n_C = m_mesh->normals()[ti4.z]; //normal c of triangle
	/*
    const Vector3 OA = r.o - A;
    const Vector3 BA = B - A;
    const Vector3 CA = C - A;

    Vector3 normal = cross(BA, CA); //Calculate the normal

    //Check to make sure ray is not parallel to triangle
    if (cross(normal, r.d) == Vector3(0)){
        return false;
    }

    //Calculate beta and gamma
	float det = 1 / (dot(-r.d, cross(BA, CA)));
	float beta = dot(-r.d, cross(OA, CA)) * det;
    float gamma = dot(-r.d, cross(BA, OA)) * det;

    //Check to see if hit inside triangle
    if ((beta < 0 || beta > 1) || (gamma < 0 || gamma > 1)){
        return false;
    }
    if ((beta + gamma > 1) || (beta + gamma < 0)){
        return false;
    }

    //Calculate t after determining whether intersect in triangle

    result.t = dot(OA, normal) / dot(-r.d, normal);

    result.P = (1-beta-gamma)*A + beta*B + gamma*C; //Barycentric coordinates
    result.N = (1 - beta - gamma)*n_A + beta*n_B + gamma*n_C; //Smooth normal
    result.N.normalize();
	*/

	//http://www.cs.virginia.edu/~gfx/Courses/2003/ImageSynthesis/papers/Acceleration/Fast%20MinimumStorage%20RayTriangle%20Intersection.pdf

	Vector3 BA = B - A;
	Vector3 CA = C - A;
	Vector3 h = cross(r.d, CA);
	float det = dot(BA, h);
	if (det > -EPSILON  && det < EPSILON)
		return false;
	float invDet = 1 / det;
	Vector3 s = r.o - A;
	float u = invDet * dot(s, h);
	if (u < 0.0 || u > 1.0)
		return false;
	Vector3 q = cross(s, BA);
	float v = invDet * dot(r.d, q);
	if (v < 0.0 || u + v > 1.0)
		return false;
	float t = invDet * dot(CA, q);
	if (t > EPSILON) {
		result.t = t;
		result.P = r.o + r.d * t;
		result.N = (1 - u - v) * n_A + u * n_B + v * n_C;
		result.N.normalize();
		result.material = this->m_material;
		return true;
	}
	return false;
}
