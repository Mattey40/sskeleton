#include "Corner.h"

using namespace boost;
using namespace std;

#include "Edge.h"
#include "Skeleton.h"

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Corner::Corner(double x, double y, double z) :
    Point3D(x, y, z)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Corner::Corner(double x, double y) :
    Point3D(x, y, 0)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Corner::Corner(Point3D in) :
    Point3D(in.x, in.y, in.z)
{
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
Point3D Corner::getLoc3(void)
{
    return Point3D(x, y, 0);
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
void Corner::replace(Corner *old, Corner *neu, Skeleton *skel)
{
    old->prevL->currentCorners.remove(old);
    old->nextL->currentCorners.remove(old);

    old->nextC->prevC = neu;
    old->prevC->nextC = neu;
    neu->prevC = old->prevC;
    neu->nextC = old->nextC;

    neu->nextL = old->nextL;
    neu->prevL = old->prevL;

    neu->prevL->currentCorners.push_back(neu);
    neu->nextL->currentCorners.push_back(neu);

    skel->liveCorners.remove(old);
    skel->liveCorners.push_back(neu);
}

/*******************************************************************************
 
  Purpose - Corners (unlike point3ds) are only equal to themselves. We never
  move a point, but can create multiple (uniques) at one location. We also
  change prev/next pointers to edges and other corners but need to retain
  hashing behaviour. Therefore we revert to the system hash.

 *******************************************************************************/
bool Corner::operator==(const Corner &other) const
{
    return this == &other;
}

/*******************************************************************************
 
  Purpose - 

 *******************************************************************************/
size_t hash_value(Corner const& e)
{
    return (size_t) &e;
}
