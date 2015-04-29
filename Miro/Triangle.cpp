#include "Triangle.h"
#include "TriangleMesh.h"
#include "Ray.h"


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

    const Vector3 OA = r.o - A;
    const Vector3 BA = B - A;
    const Vector3 CA = C - A;

    Vector3 normal = cross(BA, CA); //Calculate the normal

    //Check to make sure ray is not parallel to triangle
    if (cross(normal, r.d) == Vector3(0)){
        return false;
    }

    //Calculate beta and gamma
    float beta = dot(-r.d, cross(OA, CA)) / dot(-r.d, cross(BA, CA));
    float gamma = dot(-r.d, cross(BA, OA)) / dot(-r.d, cross(BA, CA));

    //Check to see if hit inside triangle
    if ((beta < 0 || beta > 1) || (gamma < 0 || gamma > 1)){
        return false;
    }
    if ((beta + gamma > 1) || (beta + gamma < 0)){
        return false;
    }

    //Calculate t after determining whether intersect in triangle

    result.t = dot(OA, normal) / dot(-r.d, normal);

    result.P = A + beta*B + gamma*C; //Barycentric coordinates
    result.N = (1 - beta - gamma)*n_A + beta*n_B + gamma*n_C; //Smooth normal
    result.N.normalize();
    result.material = this->m_material;

    return true;
}
