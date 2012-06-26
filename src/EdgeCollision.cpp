#include "EdgeCollision.h"

#include "CollisionQ.h"

using namespace boost;
using namespace std;

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
EdgeCollision::EdgeCollision(Point3D location, Edge *e1, Edge *e2, Edge *e3) :
    loc(location), a(e1), b(e2), c(e3)
{
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool EdgeCollision::operator==(const EdgeCollision &e) const
{
    return ((a == e.a) && (
                ((b == e.b) && (c == e.c) ) ||
                ((b == e.c) && (c == e.b)) ) ) ||
        ((a == e.b) && (
            ((b == e.a) && (c == e.c) ) ||
            ((b == e.c) && (c == e.a)) ) ) ||
        ((a == e.c) && (
            ((b == e.a) && (c == e.b)) ||
            ((b == e.b) && (c == e.a)) ) );
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
double EdgeCollision::getHeight(void) const
{
    return loc.z;
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
bool EdgeCollision::process(Skeleton *skel)
{
    throw runtime_error("three-way collisions are dealt with in "
            "CoSitedCollision");
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void EdgeCollision::processConsecutive(Point3D *loc, Corner *a, Corner *b,
        Skeleton *skel)
{
    /* add line b -> loc */
    Edge *arr[] = { b->prevL, b->nextL };
    skel->output->addOutputSideTo(b, loc, 2, arr);

    /* remove b from edge's map */
    a->nextL->currentCorners.remove(b);
    b->nextL->currentCorners.remove(b);

    skel->liveCorners.remove(b);
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
void EdgeCollision::processJump(Point3D *loc, Corner *a, Corner *an, Corner *b,
        Skeleton *skel, HeightCollision *hc)
{
    Corner *ab = GCNEW(Corner(loc->x, loc->y, loc->z));
    ab->prevL = b->nextL;
    ab->prevL->currentCorners.push_back(ab);

    ab->nextL = a->nextL;
    ab->nextL->currentCorners.push_back(ab);

    /* take's A's place in the loop */
    an->prevC = ab; // where it breaks down with an...
    b->nextC = ab;
    ab->prevC = b;
    ab->nextC = an; // ...and here

    skel->liveCorners.push_back(ab);

    /* check for new collisions */
    skel->qu->addCorner(ab, hc); 
}

/*******************************************************************************

  Purpose - returns the loop corner whose nextL points to the given edge and
  whose bisectors for the edge contain the collision.

 *******************************************************************************/
Corner * EdgeCollision::findCorner(Edge *in, Point3D collision, Skeleton *skel)
{
    IndirectLinkedHashSet<Corner *>::iterator it;
    for (it = in->currentCorners.begin(); it != in->currentCorners.end(); ++it)
    {
        Corner *lc = *it;
        if (lc->nextL == in)
        {
            /* the two edges that form the bisector with this edge */
            LinearForm3D prev = lc->prevC->nextL->linearForm;
            LinearForm3D next = lc->nextC->nextL->linearForm;

            double pDist = prev.pointDistance(collision),
                   nDist = next.pointDistance(collision);

            double prevDot = prev.normal().dot(in->direction()),
                   nextDot = next.normal().dot(in->direction());

            /* depending on if the angle is obtuse or reflex, we'll need to flip
             * the normals to the convention taht a point with a positive plane
             * distance is on the correct side of both bisecting planes
             */
            if (prevDot < 0) // should only be 0 if the two edges are parallel!
                pDist = -pDist;
            if (nextDot > 0)
                nDist = -nDist;

            /* important constant - must prefer to accept rather than "leak" a
             * collision
             */
            const double c = -0.0001;

            if (pDist >= c && nDist >= c) // a bit of slack!
                return lc;
        }
    }

    return NULL; // no candidates
}

/*******************************************************************************

  Purpose - 

 *******************************************************************************/
std::ostream & operator<<(std::ostream &strm, const EdgeCollision &c)
{
    return strm << c.loc << ":" << *c.a << "," << *c.b << "," << *c.c;
}

/*******************************************************************************

  Purpose - Hash is agnostic to which edge is in a, b and c

 *******************************************************************************/
size_t hash_value(EdgeCollision const& e)
{
    boost::hash<Edge> hasher;
    size_t hash = 3;
    hash += (e.a != NULL ? hasher(*e.a) : 0);
    hash += (e.b != NULL ? hasher(*e.b) : 0);
    hash += (e.c != NULL ? hasher(*e.c) : 0);
    return hash * 31;
}
