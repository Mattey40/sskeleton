#ifndef LINEONPLANE_H
#define LINEONPLANE_H

#include "Point3D.h"
#include "Tuple3D.h"

/**
 * A result of an intersection that is really a line
 */
class LineOnPlane: public Point3D
{
    public:
        Tuple3D direction;
        double distance;

        LineOnPlane(Tuple3D start, Tuple3D direction, double distance) :
            Point3D(start), direction(direction), distance(distance)
        {
        }
};

#endif
