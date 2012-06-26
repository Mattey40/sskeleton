#include "LinearForm3D.h"

using namespace boost;
using namespace JAMA;
using namespace TNT;
using namespace std;

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
LinearForm3D::LinearForm3D(void) :
    A(0), B(0), C(0), D(0)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
LinearForm3D::LinearForm3D(double A, double B, double C, double D) :
    A(A), B(B), C(C), D(D)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
LinearForm3D::LinearForm3D(Vector3D normal, Tuple3D offset) :
    A(normal.x), B(normal.y), C(normal.z), D(-normal.dot(Vector3D(offset)))
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Line3D LinearForm3D::collide(LinearForm3D other)
{
    /* special solution is cross product of normals */
    Vector3D spec;
    spec.cross(createNormalVector(), other.createNormalVector());

    if (abs(spec.length()) <= DBL_EPSILON) // planes are parallel
        return Line3D::invalid();

    /* particular solution can be found by solving the equation set of the two
     * planes, and another perpendicular plane
     */
    Array2D<double> matrixA(3, 3, 0.0f);
    matrixA[0][0] = A;       matrixA[0][1] = B;       matrixA[0][2] = C;
    matrixA[1][0] = other.A; matrixA[1][1] = other.B; matrixA[1][2] = other.C;
    matrixA[2][0] = spec.x;  matrixA[2][1] = spec.y;  matrixA[2][2] = spec.z;

    /* offset of perp plane can be 0, goes through the origin */
    Array2D<double> matrixB(3, 1, 0.0f);
    matrixB[0][0] = -D; matrixB[1][0] = -other.D; matrixB[2][0] = 0;

    /* Note: in C++ JAMA, we don't have Matrix.solve. Looking at the Java JAMA
     * source, it seems that because matrixA is square, we use LU decomposition.
     */
    LU<double> lu(matrixA);
    Array2D<double> res = lu.solve(matrixB);

    return Line3D(Point3D(res[0][0], res[1][0], res[2][0]), spec);
}

/*******************************************************************************
 
  Purpose - Collide three planes to a point

 *******************************************************************************/
Tuple3D LinearForm3D::collide(LinearForm3D b, LinearForm3D c)
{
    LinearForm3D a = *this;

    Array2D<double> three(3, 3, 0.0f);
    three[0][0] = a.A;      three[0][1] = a.B;      three[0][2] = a.C;
    three[1][0] = b.A;      three[1][1] = b.B;      three[1][2] = b.C;
    three[2][0] = c.A;      three[2][1] = c.B;      three[2][2] = c.C;

    Array2D<double> offset(3, 1, 0.0f);
    offset[0][0] = -a.D;    offset[1][0] = -b.D;    offset[2][0] = -c.D;

    /* See notes in collide(LinearForm3D) re: C++ JAMA */
    LU<double> lu(three);
    cout << "Matrix three det: " << lu.det() << endl;

    if (dIsApprox(lu.det(), 0))
        throw std::runtime_error("Matrix is singular");

    Array2D<double> out = lu.solve(offset);

    /* Use one point on the plane to determine the offset */
    /* what is this used for? Java code did nothing... */
   // double d =
   //     offset[0][0] * out[0][0] +
   //     offset[1][0] * out[1][0] +
   //     offset[2][0] * out[2][0];

    SVD<double> svd(three);
    if (svd.rank() != 3)
        return Tuple3D::invalid(); // not quite right, but could return a line

    return Vector3D(out[0][0], out[1][0], out[2][0]);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D LinearForm3D::collide(Tuple3D rayOrigin, Tuple3D rayDirection)
{
    return collide(rayOrigin, rayDirection, NULL);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D LinearForm3D::collide(Tuple3D rayOrigin, Tuple3D rayDirection,
                const double *distance)
{
    Vector3D direction(rayDirection);

    /* erm... dot product? */
    double den = A * direction.x + B * direction.y + C * direction.z;

    /* ray is parallel to plane, check for co-incidence */
    if (abs(den) <= DBL_EPSILON)
    {
        if (pointDistance(rayOrigin) < 0.0001) // haven't tested this yet
            return LineOnPlane(rayOrigin, rayDirection, *distance);

        return Point3D::invalid(); // not going to collide
    }

    double num = -D - A * rayOrigin.x - B * rayOrigin.y - C * rayOrigin.z;

    /* parameter n is multiple of direction vector away from origin */
    double n = num / den;

    direction.scale(n);
    direction.add(rayOrigin);

    if (n < 0)
        //return OOB(&direction); // plane too early
        return Point3D::invalid();

    if (distance != NULL && *distance != DBL_MAX)
        if (*distance < n)
            //return new OOB(&direction); // plane too late
            return Point3D::invalid();

    return Point3D(direction);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Vector3D LinearForm3D::collideToVector(LinearForm3D other)
{
    /* find the vector that occurs when both planes collide */
    Vector3D n = createNormalVector();
    n.cross(n, other.createNormalVector());

    return n;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Vector3D LinearForm3D::createNormalVector(void)
{
    Vector3D out(A, B, C);
    out.normalise();
    return out;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
double LinearForm3D::pointDistance(Tuple3D point)
{
    double den = sqrt(A * A + B * B + C * C);
    if (abs(den) <= DBL_EPSILON)
        throw runtime_error("I'm not a plane!");
    double num = A * point.x + B * point.y + C * point.z + D;
    return num/den;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool LinearForm3D::operator==(const LinearForm3D &other) const
{
    return A == other.A && B == other.B && C == other.C && D == other.D;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
Vector3D LinearForm3D::normal(void)
{
    return Vector3D(A, B, C);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool LinearForm3D::hasNaN(void)
{
    return boost::math::isnan<double>(A) ||
        boost::math::isnan<double>(B) ||
        boost::math::isnan<double>(C) ||
        boost::math::isnan<double>(D);
}
