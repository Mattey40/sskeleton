#include "Edge.h"

using namespace std;

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Edge::Edge(Corner *start, Corner *end, double angle) :
    start(start), end(end), angle(angle)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Edge::Edge(Corner *start, Corner *end) :
    start(start), end(end), angle(M_PI / 4)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Edge::Edge(Point3D start, Point3D end, double angle) :
    start(GCNEW(Corner(start))), end(GCNEW(Corner(end))), angle(angle)
{
    calculateLinearForm();
}

/*******************************************************************************
 
  Purpose - The perpendicular unit vector pointing up the slope of the side

 *******************************************************************************/
void Edge::calculateUphill(void)
{
    Vector3D vec = direction();

    /* perpendicular in x,y plane */
    vec = Vector3D(-vec.y, vec.x, 0);
    vec.normalise();


    /* horizontal component */
    vec.scale(sin(getAngle()));

    /* vertical component */
    vec.add(Vector3D(0, 0, cos(getAngle())));

    uphill = vec;
}

/*******************************************************************************
 
  Purpose -  Finds the Ax  By + Cz = D form of the edge
  Called when the weight of the edge changes

 *******************************************************************************/
void Edge::calculateLinearForm(void)
{
    calculateUphill();

    /* find normal from uphill and edge */
    Vector3D norm = getPlaneNormal();

    linearForm = LinearForm3D(norm, Tuple3D(start->x, start->y, start->z));
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Vector3D Edge::direction(void)
{
    Vector3D vec(end->x, end->y, 0);
    vec.sub(Tuple3D(start->x, start->y, 0));
    return vec;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
bool Edge::isCollisionNearHoriz(Edge *other)
{
    return abs(linearForm.collide(other->linearForm).direction.z) < 0.001;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
bool Edge::sameDirectedLine(Edge *nextL)
{
    return nextL->direction().angle(direction()) < 0.01 &&
        abs(getAngle() - nextL->getAngle()) < 0.01;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
double Edge::getAngle(void)
{
    return angle;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
void Edge::setAngle(double angle)
{
    this->angle = angle;
    calculateLinearForm();
}

/*******************************************************************************
 
  Purpose -  The normal the edge

 *******************************************************************************/
Vector3D Edge::getPlaneNormal(void)
{
    Vector3D a = direction();
    a.normalise();
    a.cross(a, uphill);
    return a;
}

/*******************************************************************************
 
  Purpose - Copying Java's default .equals method; potentially a little bit
  dodgy (one edge therefore only equals another edge if they are actually
  exactly the same instance).

 *******************************************************************************/
bool Edge::operator==(const Edge &other) const
{
    return this == &other;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
bool Edge::operator!=(const Edge &other) const
{
    return !(*this == other);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const Edge &e)
{
    return strm << "[" << *e.start << "," << *e.end << "]";
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Tuple3D Edge::collide(Corner *a, double height)
{
    LinearForm3D ceiling(0, 0, 1, -height);

    /* this can cause Jama not to return... */
    if (a->prevL->linearForm.hasNaN() || a->nextL->linearForm.hasNaN())
        throw runtime_error("linearform has NaN!");

    try
    {
        return ceiling.collide(a->prevL->linearForm, a->nextL->linearForm);
    }
    catch (exception &e)
    {
        if (!a->prevL->sameDirectedLine(a->nextL))
            throw runtime_error("should be same line but isn't!");

        /* a vector in the direction of uphill from a */
        Vector3D dir(a->prevL->uphill);
        dir.normalise();
        /* via similar triangle (pyramids) */
        dir.scale(height - a->z);
        dir.add(*a);

        /* assume they're all coincident? */
        return Point3D(dir.x, dir.y, height);
    }
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
size_t hash_value(const Edge &s)
{
    return (size_t) &s;
}
