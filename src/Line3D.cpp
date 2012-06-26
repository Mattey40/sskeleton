#include "Line3D.h"

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Line3D::Line3D(void)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Line3D::Line3D(Tuple3D origin, Tuple3D direction) :
    origin(Point3D(origin)), direction(Vector3D(direction))
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D Line3D::projectLine(Point3D ept)
{
    return project(ept, false);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D Line3D::projectSegment(Point3D ept)
{
    return project(ept, true);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
double Line3D::projectParam(const Tuple3D ept) const
{
    Vector3D b(ept);
    b.sub(origin);
    double factor = direction.dot(b) / direction.dot(direction);
    return factor;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D Line3D::project(Point3D ept, bool segment)
{
    double factor = projectParam(ept);
    if (segment && (factor < 0 || factor > 1))
        return Point3D::invalid();
    Point3D dest(direction);
    dest.scale(factor);
    dest.add(origin);
    return dest;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const Line3D &l)
{
    return strm << "[" << l.origin << "." << l.direction << "]";
}
