#ifndef ANGLEACCUMULATOR_H
#define ANGLEACCUMULATOR_H

#include <cmath>
#include <iostream>
#include "Point3D.h"
#include "Vector3D.h"

/**
 * Keeps track of the angles of a polygon to decide on clockwise or
 * counterclockwise determination. Should assume anti-clockwise traversal.
 */
class AngleAccumulator
{
    public:
        AngleAccumulator(bool positive, Vector3D normal);
        ~AngleAccumulator(void);
        void add(Point3D pt);
        void add(Vector3D v);
        bool correctAngle(void);

    private:
        bool targetPositive;
        double angle;
        Point3D *firstPoint;
        Point3D *lastPoint;
        Vector3D *firstVector;
        Vector3D *lastVector;
        Vector3D *normal;
};

#endif
